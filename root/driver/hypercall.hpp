#pragma once

#include <cstdint>
#include <cstddef>

namespace hypercall
{
    extern "C" uint64_t launch_raw_hypercall(uint64_t hypercall_info, uint64_t rdx, uint64_t r8, uint64_t r9);

    uint64_t read_guest_physical_memory(void* dst, uint64_t guest_source_physical_address, uint64_t size);
    uint64_t write_guest_physical_memory(const void* src, uint64_t guest_destination_physical_address, uint64_t size);

    uint64_t read_guest_virtual_memory(void* dst, uint64_t guest_source_virtual_address, uint64_t guest_source_cr3, uint64_t size);
    uint64_t write_guest_virtual_memory(const void* src, uint64_t guest_destination_virtual_address, uint64_t guest_destination_cr3, uint64_t size);

    uint64_t translate_guest_virtual_address(uint64_t guest_source_virtual_address, uint64_t guest_source_cr3);
    uint64_t read_guest_cr3(void);

    uint64_t add_slat_code_hook(uint64_t target_guest_physical_address, uint64_t shadow_guest_physical_address);
    uint64_t remove_slat_code_hook(uint64_t target_guest_physical_address);
    uint64_t hide_guest_physical_page(uint64_t guest_physical_address);
    uint64_t get_heap_free_page_count(void);
} 