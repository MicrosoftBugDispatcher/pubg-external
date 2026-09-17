#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace hvre
{
    struct module_t
    {
        std::string name;
        uint64_t base = 0;
        uint64_t size = 0;
    };

    
    inline bool available = false;
    inline bool attached = false;
    inline uint64_t own_cr3 = 0;
    inline uint64_t target_pid = 0;
    inline uint64_t target_cr3 = 0;
    inline uint64_t target_base = 0;
    inline std::string target_process_name;
    inline std::vector<module_t> modules;

    
    
    bool initialize(void);

    
    
    bool bind(const std::string& process_name);

    
    
    bool refresh_modules(void);

    uint64_t get_pid(const std::string& name);

    
    module_t get_module(const std::string& name);

    
    bool read(void* dst, uint64_t src, uint64_t size);
    bool write(uint64_t dst, const void* src, uint64_t size);
    uint64_t translate(uint64_t va, uint64_t cr3);

    
    uint64_t get_ntoskrnl_base(void);
    uint64_t find_mmpfn_database(void);
    uint64_t bruteforce_cr3(uint64_t base_address, uint64_t mm_pfn_database);
} 