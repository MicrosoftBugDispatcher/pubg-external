#pragma once

#include <cstdint>

namespace hypercall
{
    enum class memory_operation_t : uint64_t
    {
        read_operation,
        write_operation
    };

    enum class hypercall_type_t : uint64_t
    {
        guest_physical_memory_operation,
        guest_virtual_memory_operation,
        translate_guest_virtual_address,
        read_guest_cr3,
        add_slat_code_hook,
        remove_slat_code_hook,
        hide_guest_physical_page,
        log_current_state,
        flush_logs,
        get_heap_free_page_count
    };

    constexpr uint64_t hypercall_primary_key = 0x4E47;
    constexpr uint64_t hypercall_secondary_key = 0x7F;

    
#pragma warning(push)
#pragma warning(disable: 4201)

    union hypercall_info_t
    {
        uint64_t value;

        struct
        {
            uint64_t primary_key : 16;
            hypercall::hypercall_type_t call_type : 4;
            uint64_t secondary_key : 7;
            uint64_t call_reserved_data : 37;
        };
    };

    union virt_memory_op_hypercall_info_t
    {
        uint64_t value;

        struct
        {
            uint64_t primary_key : 16;
            hypercall::hypercall_type_t call_type : 4;
            uint64_t secondary_key : 7;
            hypercall::memory_operation_t memory_operation : 1;
            uint64_t address_of_page_directory : 36;
        };
    };

#pragma warning(pop)
} 