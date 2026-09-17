#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <optional>
#include "../additional/logger/logger.cuh"

// Simple driver interface using HVRE namespace directly
class driver_t
{
public:
	INT32 target_pid = 0;
	ULONGLONG target_cr3 = 0;
	uintptr_t module_base = 0;
	std::wstring target_process_name = L"";

private:
	bool initialized = false;

public:
	bool connect_to_driver()
	{
		if (initialized) {
			return true;
		}

		// Use HVRE namespace directly
		initialized = true;
		return true;
	}

	void disconnect()
	{
		if (!initialized) {
			return;
		}

		initialized = false;
	}

	INT32 find_process_id_by_name(const wchar_t* name)
	{
		// Not implemented - will use HVRE directly
		return 0;
	}

	ULONGLONG get_process_cr3()
	{
		if (!initialized) return 0;

		return target_cr3;
	}

	bool read_physical_memory(uintptr_t address, void* buffer, DWORD size)
	{
		// Use virtual memory read
		return read_virtual_memory(address, buffer, size);
	}

	bool write_physical_memory(uintptr_t address, const void* buffer, DWORD size)
	{
		// Use virtual memory write
		return write_virtual_memory(address, buffer, size);
	}

	// Virtual memory operations using HVRE
	bool read_virtual_memory(uintptr_t address, void* buffer, DWORD size)
	{
		if (!initialized || !buffer || !size || !address) return false;

		// Placeholder - actual implementation would use HVRE
		memset(buffer, 0, size);
		return false;
	}

	bool write_virtual_memory(uintptr_t address, const void* buffer, DWORD size)
	{
		if (!initialized || !buffer || !size || !address) return false;

		// Placeholder - actual implementation would use HVRE
		return false;
	}

	uintptr_t get_module_base_address()
	{
		if (!initialized) return 0;

		return module_base;
	}

	uintptr_t get_guarded_region_address()
	{
		return 0;
	}

	std::wstring get_attached_process_name() const
	{
		return target_process_name;
	}

	bool attach_to_process(const std::wstring& name)
	{
		if (!connect_to_driver()) {
			return false;
		}

		target_process_name = name;

		// Placeholder - actual implementation would use HVRE
		target_pid = 0;
		module_base = 0;
		target_cr3 = 0;

		return true;
	}

	template<typename T>
	T read(uint64_t address)
	{
		T value{};
		read_virtual_memory(address, &value, sizeof(T));
		return value;
	}

	template<typename T>
	bool write(uint64_t address, const T& value)
	{
		return write_virtual_memory(address, &value, sizeof(T));
	}

	bool move_mouse(LONG delta_x = 0, LONG delta_y = 0, USHORT button_flags = 0)
	{
		if (!initialized) {
			return false;
		}

		return false;
	}

	std::string read_string_raw(uintptr_t address, std::size_t max_length = 200)
	{
		std::string output;
		output.reserve(max_length);
		for (std::size_t i = 0; i < max_length; i++)
		{
			const auto ch = read<char>(address + i);
			if (!ch) break;
			output.push_back(ch);
		}
		return output;
	}

	template<typename T>
	T read_chain(uintptr_t address, const std::vector<uintptr_t>& offsets)
	{
		auto current = address;
		for (std::size_t i = 0; i + 1 < offsets.size(); i++)
			current = read<uintptr_t>(current + offsets[i]);
		return read<T>(current + offsets.back());
	}
};

inline std::unique_ptr<driver_t> driver = std::make_unique<driver_t>();
inline bool write_buffer(uint64_t address, const void* buffer, size_t size)
{
	return driver->write_virtual_memory(address, buffer, static_cast<DWORD>(size));
}
