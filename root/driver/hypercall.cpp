#include "hypercall.hpp"
#include "hypercall_def.h"

namespace hypercall
{
    static uint64_t make_hypercall(const hypercall::hypercall_type_t call_type, const uint64_t call_reserved_data,
                                   const uint64_t rdx, const uint64_t r8, const uint64_t r9)
    {
        hypercall::hypercall_info_t hypercall_info;
        hypercall_info.value = 0;

        hypercall_info.primary_key = hypercall::hypercall_primary_key;
        hypercall_info.secondary_key = hypercall::hypercall_secondary_key;
        hypercall_info.call_type = call_type;
        hypercall_info.call_reserved_data = call_reserved_data;

        return hypercall::launch_raw_hypercall(hypercall_info.value, rdx, r8, r9);
    }

    uint64_t read_guest_physical_memory(void* dst, uint64_t guest_source_physical_address, uint64_t size)
    {
        constexpr auto call_type = hypercall::hypercall_type_t::guest_physical_memory_operation;
        constexpr auto operation_type = static_cast<uint64_t>(hypercall::memory_operation_t::read_operation);

        const auto guest_destination_virtual_address = reinterpret_cast<uint64_t>(dst);

        return hypercall::make_hypercall(call_type, operation_type, guest_source_physical_address,
                                         guest_destination_virtual_address, size);
    }

    uint64_t write_guest_physical_memory(const void* src, uint64_t guest_destination_physical_address, uint64_t size)
    {
        constexpr auto call_type = hypercall::hypercall_type_t::guest_physical_memory_operation;
        constexpr auto operation_type = static_cast<uint64_t>(hypercall::memory_operation_t::write_operation);

        const auto guest_source_virtual_address = reinterpret_cast<uint64_t>(src);

        return hypercall::make_hypercall(call_type, operation_type, guest_destination_physical_address,
                                         guest_source_virtual_address, size);
    }

    uint64_t read_guest_virtual_memory(void* dst, uint64_t guest_source_virtual_address, uint64_t source_cr3,
                                       uint64_t size)
    {
        hypercall::virt_memory_op_hypercall_info_t memory_op_call;
        memory_op_call.value = 0;

        memory_op_call.call_type = hypercall::hypercall_type_t::guest_virtual_memory_operation;
        memory_op_call.memory_operation = hypercall::memory_operation_t::read_operation;
        memory_op_call.address_of_page_directory = source_cr3 >> 12;

        const hypercall::hypercall_info_t hypercall_info = {.value = memory_op_call.value};
        const auto guest_destination_virtual_address = reinterpret_cast<uint64_t>(dst);

        return hypercall::make_hypercall(hypercall_info.call_type, hypercall_info.call_reserved_data,
                                         guest_destination_virtual_address, guest_source_virtual_address, size);
    }

    uint64_t write_guest_virtual_memory(const void* src, uint64_t guest_destination_virtual_address,
                                        uint64_t destination_cr3, uint64_t size)
    {
        hypercall::virt_memory_op_hypercall_info_t memory_op_call;
        memory_op_call.value = 0;

        memory_op_call.call_type = hypercall::hypercall_type_t::guest_virtual_memory_operation;
        memory_op_call.memory_operation = hypercall::memory_operation_t::write_operation;
        memory_op_call.address_of_page_directory = destination_cr3 >> 12;

        const hypercall::hypercall_info_t hypercall_info = {.value = memory_op_call.value};
        const auto guest_source_virtual_address = reinterpret_cast<uint64_t>(src);

        return hypercall::make_hypercall(hypercall_info.call_type, hypercall_info.call_reserved_data,
                                         guest_source_virtual_address, guest_destination_virtual_address, size);
    }

    uint64_t translate_guest_virtual_address(uint64_t guest_virtual_address, uint64_t guest_cr3)
    {
        constexpr auto call_type = hypercall::hypercall_type_t::translate_guest_virtual_address;

        return hypercall::make_hypercall(call_type, 0, guest_virtual_address, guest_cr3, 0);
    }

    uint64_t read_guest_cr3(void)
    {
        constexpr auto call_type = hypercall::hypercall_type_t::read_guest_cr3;

        return hypercall::make_hypercall(call_type, 0, 0, 0, 0);
    }

    uint64_t add_slat_code_hook(uint64_t target_guest_physical_address, uint64_t shadow_page_guest_physical_address)
    {
        constexpr auto call_type = hypercall::hypercall_type_t::add_slat_code_hook;

        return hypercall::make_hypercall(call_type, 0, target_guest_physical_address,
                                         shadow_page_guest_physical_address, 0);
    }

    uint64_t remove_slat_code_hook(uint64_t target_guest_physical_address)
    {
        constexpr auto call_type = hypercall::hypercall_type_t::remove_slat_code_hook;

        return hypercall::make_hypercall(call_type, 0, target_guest_physical_address, 0, 0);
    }

    uint64_t hide_guest_physical_page(uint64_t target_guest_physical_address)
    {
        constexpr auto call_type = hypercall::hypercall_type_t::hide_guest_physical_page;

        return hypercall::make_hypercall(call_type, 0, target_guest_physical_address, 0, 0);
    }

    uint64_t get_heap_free_page_count(void)
    {
        constexpr auto call_type = hypercall::hypercall_type_t::get_heap_free_page_count;

        return hypercall::make_hypercall(call_type, 0, 0, 0, 0);
    }
} 