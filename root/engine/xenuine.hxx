#pragma once
#include <cstdint>
#include <intrin.h>

namespace xenuine
{
	// Initialize the Xenuine decryptor by parsing the game's decryption function
	auto setup(uint64_t fn_ptr_va, const uint8_t* bytes) -> bool;

	// Decrypt an encrypted pointer
	auto decrypt_pointer(uint64_t encrypted) -> uint64_t;

	// Decrypt CIndex (for GNames)
	auto decrypt_c_index(uint32_t value) -> uint32_t;

	// Check if decryptor is ready
	auto is_ready() -> bool;
}
