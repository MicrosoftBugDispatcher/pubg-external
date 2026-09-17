#include "xenuine.hxx"
#include "../additional/logger/logger.cuh"

namespace xenuine
{
	typedef enum
	{
		OP_XOR,
		OP_ADD,
		OP_SUB,
		OP_ROL_64,
		OP_ROL_8
	} OpType;

	typedef struct
	{
		OpType type;
		uint64_t val;
	} Step;

	constexpr int MAX_STEPS = 32;
	static Step g_steps[MAX_STEPS];
	static int g_step_count = 0;
	static uint64_t g_rax_key = 0;
	static bool g_ready = false;

	auto setup(uint64_t fn_ptr_va, const uint8_t* bytes) -> bool
	{
		if (!bytes)
			return false;

		g_step_count = 0;
		g_rax_key = 0;
		g_ready = false;

		logger->print("[Xe] Setup() starting - fnVA=0x%016llX", fn_ptr_va);

		// LEA RAX, [RIP + off] (48 8D 05 ?? ?? ?? ??)
		if (bytes[0] == 0x48 && bytes[1] == 0x8D && bytes[2] == 0x05)
		{
			int32_t off = *(const int32_t*)(bytes + 3);
			g_rax_key = fn_ptr_va + 7 + (int64_t)off;
			logger->print("[Xe]   prologue LEA RAX,[RIP+0x%X] -> rax_key=0x%016llX",
				(unsigned int)off, g_rax_key);
		}
		else
		{
			logger->print("[Xe]   WARNING: prologue doesn't start with LEA RAX,[RIP+off] (48 8D 05). Bytes[0..2]=%02X %02X %02X",
				bytes[0], bytes[1], bytes[2]);
		}

		// Scan up to 80 bytes or until RET (0xC3).
		for (int i = 0; i < 80;)
		{
			if (bytes[i] == 0xC3)
			{
				logger->print("[Xe]   off=0x%02X RET (0xC3) - end of function", i);
				break;
			}

			// XOR RCX, imm32  (48 81 F1 ?? ?? ?? ??)
			if (bytes[i] == 0x48 && bytes[i + 1] == 0x81 && bytes[i + 2] == 0xF1)
			{
				int32_t imm = *(const int32_t*)(bytes + i + 3);
				uint64_t val = (uint64_t)(int64_t)imm;
				if (g_step_count < MAX_STEPS)
				{
					g_steps[g_step_count].type = OP_XOR;
					g_steps[g_step_count].val = val;
				}
				logger->print("[Xe]   off=0x%02X step[%d] XOR      RCX, 0x%08X",
					i, g_step_count, (unsigned int)imm);
				g_step_count++;
				i += 7;
				continue;
			}

			// ADD RCX, imm32  (48 81 C1 ?? ?? ?? ??)
			if (bytes[i] == 0x48 && bytes[i + 1] == 0x81 && bytes[i + 2] == 0xC1)
			{
				int32_t imm = *(const int32_t*)(bytes + i + 3);
				uint64_t val = (uint64_t)(int64_t)imm;
				if (g_step_count < MAX_STEPS)
				{
					g_steps[g_step_count].type = OP_ADD;
					g_steps[g_step_count].val = val;
				}
				logger->print("[Xe]   off=0x%02X step[%d] ADD      RCX, 0x%08X",
					i, g_step_count, (unsigned int)imm);
				g_step_count++;
				i += 7;
				continue;
			}

			// SUB RCX, RAX    (48 29 C1)
			if (bytes[i] == 0x48 && bytes[i + 1] == 0x29 && bytes[i + 2] == 0xC1)
			{
				if (g_step_count < MAX_STEPS)
				{
					g_steps[g_step_count].type = OP_SUB;
					g_steps[g_step_count].val = g_rax_key;
				}
				logger->print("[Xe]   off=0x%02X step[%d] SUB      RCX, RAX (rax_key=0x%016llX)",
					i, g_step_count, g_rax_key);
				g_step_count++;
				i += 3;
				continue;
			}

			// ROL RCX, imm8   (48 C1 C1 ??)
			if (bytes[i] == 0x48 && bytes[i + 1] == 0xC1 && bytes[i + 2] == 0xC1)
			{
				uint8_t imm = bytes[i + 3];
				if (g_step_count < MAX_STEPS)
				{
					g_steps[g_step_count].type = OP_ROL_64;
					g_steps[g_step_count].val = (uint64_t)imm;
				}
				logger->print("[Xe]   off=0x%02X step[%d] ROL_64  RCX, %u",
					i, g_step_count, imm);
				g_step_count++;
				i += 4;
				continue;
			}

			// ROL CL, imm8    (C0 C1 ??)
			if (bytes[i] == 0xC0 && bytes[i + 1] == 0xC1)
			{
				uint8_t imm = bytes[i + 2];
				if (g_step_count < MAX_STEPS)
				{
					g_steps[g_step_count].type = OP_ROL_8;
					g_steps[g_step_count].val = (uint64_t)imm;
				}
				logger->print("[Xe]   off=0x%02X step[%d] ROL_8   CL,  %u",
					i, g_step_count, imm);
				g_step_count++;
				i += 3;
				continue;
			}

			// Unrecognized - skip 1 byte.
			logger->print("[Xe]   off=0x%02X ??  byte=0x%02X (unrecognized instruction)",
				i, bytes[i]);
			i += 1;
		}

		g_ready = (g_step_count > 0);
		logger->print("[Xe] Setup() completed: %d step(s), ready=%s",
			g_step_count, g_ready ? "true" : "false");

		return g_ready;
	}

	auto decrypt_pointer(uint64_t encrypted) -> uint64_t
	{
		if (!g_ready || encrypted == 0)
			return 0;

		uint64_t res = encrypted;
		for (int i = 0; i < g_step_count; i++)
		{
			switch (g_steps[i].type)
			{
			case OP_XOR:
				res ^= g_steps[i].val;
				break;
			case OP_ADD:
				res += g_steps[i].val;
				break;
			case OP_SUB:
				res -= g_steps[i].val;
				break;
			case OP_ROL_64:
				res = _rotl64(res, (int)(g_steps[i].val & 63));
				break;
			case OP_ROL_8:
			{
				uint8_t cl = (uint8_t)(res & 0xFF);
				int amt = (int)(g_steps[i].val & 7);
				cl = (uint8_t)((cl << amt) | (cl >> (8 - amt)));
				res = (res & 0xFFFFFFFFFFFFFF00ULL) | cl;
				break;
			}
			}
		}
		return res;
	}

	auto decrypt_c_index(uint32_t value) -> uint32_t
	{
		if (value == 0)
			return 0;

		uint32_t xor1 = value ^ 0x7360F24u;
		uint32_t part1 = (xor1 << 25) | ((xor1 >> 7) & 0x1FF0000u);
		uint32_t part2 = _rotr(xor1, 23);
		return part1 ^ part2 ^ 0xB621EC05u;
	}

	auto is_ready() -> bool
	{
		return g_ready;
	}
}
