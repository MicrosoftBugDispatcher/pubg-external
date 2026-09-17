#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <winternl.h>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>

#include "hvre.hpp"
#include "hypercall.hpp"

#pragma comment(lib, "psapi.lib")

namespace hvre
{
    
    struct eprocess_offsets_t
    {
        uint64_t directory_table_base;  
        uint64_t exit_time;
        uint64_t unique_process_id;     
        uint64_t active_process_links;  
        uint64_t section_base_address;  
        uint64_t peb;                   
        uint64_t se_audit_process_creation_info;
    };

    
    static constexpr eprocess_offsets_t eprocess_win10_1507 = { 0x028, 0x670, 0x2E8, 0x2F0, 0x3C0, 0x3F8, 0x460 }; 
    static constexpr eprocess_offsets_t eprocess_win10_1511 = { 0x028, 0x678, 0x2E8, 0x2F0, 0x3C0, 0x3F8, 0x468 }; 
    static constexpr eprocess_offsets_t eprocess_win10_1607 = { 0x028, 0x688, 0x2E8, 0x2F0, 0x3C0, 0x3F8, 0x468 }; 
    static constexpr eprocess_offsets_t eprocess_win10_1703 = { 0x028, 0x690, 0x2E0, 0x2E8, 0x3C0, 0x3F8, 0x468 }; 
    static constexpr eprocess_offsets_t eprocess_win10_1709 = { 0x028, 0x690, 0x2E0, 0x2E8, 0x3C0, 0x3F8, 0x468 }; 
    static constexpr eprocess_offsets_t eprocess_win10_1903 = { 0x028, 0x6C0, 0x2E8, 0x2F0, 0x3C8, 0x3F8, 0x468 }; 
    static constexpr eprocess_offsets_t eprocess_win10_2004 = { 0x028, 0x840, 0x440, 0x448, 0x520, 0x550, 0x5C0 }; 
    static constexpr eprocess_offsets_t eprocess_win11_24H2 = { 0x028, 0x5C0, 0x1D0, 0x1D8, 0x2B0, 0x2E0, 0x350 }; 

    static eprocess_offsets_t eprocess_offsets = {};
    static bool eprocess_offsets_ready = false;

    static bool init_eprocess_offsets(void)
    {
        if (eprocess_offsets_ready)
            return true;

        using RtlGetVersionFn = NTSTATUS(NTAPI*)(PRTL_OSVERSIONINFOW);
        const auto RtlGetVersion = reinterpret_cast<RtlGetVersionFn>(
            GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlGetVersion"));
        if (!RtlGetVersion)
            return false;

        OSVERSIONINFOEXW ovi = {};
        
        if (!NT_SUCCESS(RtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOW>(&ovi))))
            return false;

        const uint32_t build = ovi.dwBuildNumber;

        if (build >= 26100)
            eprocess_offsets = eprocess_win11_24H2;
        else if (build >= 19041)
            eprocess_offsets = eprocess_win10_2004;
        else if (build >= 18362)
            eprocess_offsets = eprocess_win10_1903;
        else if (build >= 15063)
            eprocess_offsets = (build >= 16299) ? eprocess_win10_1709 : eprocess_win10_1703;
        else if (build >= 14393)
            eprocess_offsets = eprocess_win10_1607;
        else if (build >= 10586)
            eprocess_offsets = eprocess_win10_1511;
        else if (build >= 10240)
            eprocess_offsets = eprocess_win10_1507;
        else
            return false;

        eprocess_offsets_ready = true;
        return true;
    }

    uint64_t read_own_cr3(void)
    {
        const uint64_t guest_cr3 = hypercall::read_guest_cr3();
        if (guest_cr3 <= 0x1000)
            return 0;
        return guest_cr3;
    }

    uint64_t get_ntoskrnl_base(void)
    {
        static uint64_t cached = 0;
        if (cached)
            return cached;

        std::vector<LPVOID> drivers(1024);
        DWORD needed = 0;
        if (!EnumDeviceDrivers(drivers.data(), static_cast<DWORD>(drivers.size() * sizeof(LPVOID)), &needed))
            return 0;

        const size_t count = needed / sizeof(LPVOID);
        for (size_t i = 0; i < count; i++)
        {
            char base_name[MAX_PATH]{};
            if (!GetDeviceDriverBaseNameA(drivers[i], base_name, sizeof(base_name)))
                continue;
            if (_stricmp(base_name, "ntoskrnl.exe") == 0)
            {
                cached = reinterpret_cast<uint64_t>(drivers[i]);
                return cached;
            }
        }
        return 0;
    }

    static uint64_t read_exported_function(uint64_t module_base, const char* function_name)
    {
        
        uint8_t headers[0x1000]{};
        if (hypercall::read_guest_virtual_memory(headers, module_base, own_cr3, sizeof(headers)) != sizeof(headers))
            return 0;

        const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(headers);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE)
            return 0;

        const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(headers + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE || nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
            return 0;

        const IMAGE_DATA_DIRECTORY& export_dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
        if (!export_dir.VirtualAddress || !export_dir.Size)
            return 0;

        
        std::vector<uint8_t> ex_dir(export_dir.Size);
        if (hypercall::read_guest_virtual_memory(ex_dir.data(), module_base + export_dir.VirtualAddress, own_cr3,
                                                 export_dir.Size) != export_dir.Size)
            return 0;

        const auto* exp = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(ex_dir.data());
        if (!exp->NumberOfNames || !exp->AddressOfFunctions || !exp->AddressOfNames || !exp->AddressOfNameOrdinals)
            return 0;

        
        const auto resolve_rva = [&](uint32_t rva) -> const uint8_t*
        {
            const uint32_t offset = rva - export_dir.VirtualAddress;
            if (offset >= export_dir.Size)
                return nullptr;
            return ex_dir.data() + offset;
        };

        const auto* names = reinterpret_cast<const uint32_t*>(resolve_rva(exp->AddressOfNames));
        const auto* ordinals = reinterpret_cast<const uint16_t*>(resolve_rva(exp->AddressOfNameOrdinals));
        const auto* functions = reinterpret_cast<const uint32_t*>(resolve_rva(exp->AddressOfFunctions));
        if (!names || !ordinals || !functions)
            return 0;

        for (uint32_t i = 0; i < exp->NumberOfNames; i++)
        {
            const uint8_t* name_ptr = resolve_rva(names[i]);
            if (!name_ptr)
                continue;

            if (strcmp(reinterpret_cast<const char*>(name_ptr), function_name) == 0)
            {
                const uint32_t ordinal = ordinals[i];
                return module_base + functions[ordinal];
            }
        }

        return 0;
    }

    uint64_t find_mmpfn_database(void)
    {
        static uint64_t cached = 0;
        if (cached)
            return cached;

        const uint64_t ntoskrnl = get_ntoskrnl_base();
        if (!ntoskrnl)
            return 0;

        const uint64_t func_addr = read_exported_function(ntoskrnl, "MmGetVirtualForPhysical");
        if (!func_addr)
            return 0;

        uint8_t code[0x30]{};
        if (hypercall::read_guest_virtual_memory(code, func_addr, own_cr3, sizeof(code)) != sizeof(code))
            return 0;

        
        static const uint8_t pattern[] = {0x48, 0x8B, 0xC1, 0x48, 0xC1, 0xE8, 0x0C, 0x48, 0x8D, 0x14,
                                          0x40, 0x48, 0x03, 0xD2, 0x48, 0xB8};

        for (size_t i = 0; i + sizeof(pattern) + 8 <= sizeof(code); i++)
        {
            if (memcmp(code + i, pattern, sizeof(pattern)) == 0)
            {
                uint64_t pfn_db = 0;
                memcpy(&pfn_db, code + i + sizeof(pattern), sizeof(pfn_db));
                cached = pfn_db & ~0xFFFULL;
                return cached;
            }
        }

        return 0;
    }

    uint64_t bruteforce_cr3(uint64_t base_address, uint64_t mm_pfn_database)
    {
        if (!mm_pfn_database)
            return 0;

        MEMORYSTATUSEX mem_info = {};
        mem_info.dwLength = sizeof(mem_info);
        if (!GlobalMemoryStatusEx(&mem_info))
            return 0;

        const uint64_t max_pfn = (mem_info.ullTotalPhys + 0xFFF) >> 12;

        constexpr uint64_t MMPFN_SIZE = 0x30;
        constexpr uint64_t PTE_FRAME_OFFSET = 0x28;
        constexpr uint64_t PTE_FRAME_MASK = 0xFFFFFFFFFFULL;
        constexpr int BATCH_SIZE = 512;

        std::vector<uint8_t> pfn_batch(static_cast<size_t>(BATCH_SIZE) * MMPFN_SIZE);

        for (uint64_t pfn_base = 1; pfn_base < max_pfn; pfn_base += BATCH_SIZE)
        {
            const uint64_t count = (max_pfn - pfn_base < BATCH_SIZE) ? (max_pfn - pfn_base) : BATCH_SIZE;
            const uint64_t read_size = count * MMPFN_SIZE;
            const uint64_t read_addr = mm_pfn_database + pfn_base * MMPFN_SIZE;

            if (hypercall::read_guest_virtual_memory(pfn_batch.data(), read_addr, own_cr3, read_size) != read_size)
                continue;

            for (uint64_t i = 0; i < count; i++)
            {
                const uint64_t pfn = pfn_base + i;

                uint64_t u4 = 0;
                memcpy(&u4, pfn_batch.data() + i * MMPFN_SIZE + PTE_FRAME_OFFSET, sizeof(u4));
                const uint64_t pte_frame = u4 & PTE_FRAME_MASK;

                if (pte_frame != pfn)
                    continue;

                const uint64_t candidate_cr3 = pfn << 12;

                uint16_t magic = 0;
                if (hypercall::read_guest_virtual_memory(&magic, base_address, candidate_cr3, sizeof(magic)) ==
                        sizeof(magic) &&
                    magic == IMAGE_DOS_SIGNATURE)
                {
                    return candidate_cr3;
                }
            }
        }

        return 0;
    }

    template <class t>
    static t read_kernel_virtual_memory(uint64_t address)
    {
        t buffer = t();
        if (hypercall::read_guest_virtual_memory(&buffer, address, own_cr3, sizeof(t)) != sizeof(t))
            return t();
        return buffer;
    }

    static uint64_t get_ps_initial_system_process(void)
    {
        const uint64_t ntoskrnl = get_ntoskrnl_base();
        if (!ntoskrnl)
            return 0;
        return read_exported_function(ntoskrnl, "PsInitialSystemProcess");
    }

    
    static uint64_t find_peb(uint64_t pid, uint64_t* out_base)
    {
        const uint64_t ps_initial = get_ps_initial_system_process();
        if (!ps_initial)
            return 0;

        uint64_t current_entry = read_kernel_virtual_memory<uint64_t>(ps_initial);
        if (!current_entry)
            return 0;

        const uint64_t start_entry = current_entry;
        const auto& o = eprocess_offsets;

        int guard = 0;
        do
        {
            if (++guard > 1024)
                break;

            const uint64_t flink = read_kernel_virtual_memory<uint64_t>(current_entry + o.active_process_links);
            if (!flink)
                break;
            const uint64_t next_entry = flink - o.active_process_links;

            const uint64_t exit_time = read_kernel_virtual_memory<uint64_t>(current_entry + o.exit_time);
            if (exit_time == 0)
            {
                const uint64_t entry_pid = read_kernel_virtual_memory<uint64_t>(current_entry + o.unique_process_id);
                if (entry_pid == pid)
                {
                    if (out_base)
                        *out_base = read_kernel_virtual_memory<uint64_t>(current_entry + o.section_base_address);
                    return read_kernel_virtual_memory<uint64_t>(current_entry + o.peb);
                }
            }

            current_entry = next_entry;
        } while (current_entry != start_entry);

        return 0;
    }

    
    
    static bool snapshot_modules_from_peb(uint64_t peb)
    {
        if (!peb || !target_cr3)
            return false;

        modules.clear();

        uint64_t ldr = 0;
        if (hypercall::read_guest_virtual_memory(&ldr, peb + 0x18, target_cr3, sizeof(ldr)) != sizeof(ldr))
            return false;
        if (!ldr)
            return false;

        const uint64_t list_head = ldr + 0x10; 
        uint64_t current_entry = 0;
        if (hypercall::read_guest_virtual_memory(&current_entry, list_head, target_cr3, sizeof(current_entry)) != sizeof(current_entry))
            return false;

        int guard = 0;
        while (current_entry != list_head)
        {
            if (++guard > 4096)
                break;

            uint64_t base = 0, size = 0;
            uint64_t name_ptr = 0;
            unsigned short name_len = 0;

            if (hypercall::read_guest_virtual_memory(&base, current_entry + 0x30, target_cr3, sizeof(base)) != sizeof(base)) break;
            if (hypercall::read_guest_virtual_memory(&size, current_entry + 0x40, target_cr3, sizeof(size)) != sizeof(size)) break;
            if (hypercall::read_guest_virtual_memory(&name_ptr, current_entry + 0x58 + 8, target_cr3, sizeof(name_ptr)) != sizeof(name_ptr)) break;
            if (hypercall::read_guest_virtual_memory(&name_len, current_entry + 0x58, target_cr3, sizeof(name_len)) != sizeof(name_len)) break;

            if (name_ptr && name_len)
            {
                std::wstring wname(name_len / 2, L'\0');
                if (hypercall::read_guest_virtual_memory(wname.data(), name_ptr, target_cr3, name_len) == name_len)
                {
                    char name_buf[MAX_PATH]{};
                    WideCharToMultiByte(CP_UTF8, 0, wname.c_str(), -1, name_buf, sizeof(name_buf), nullptr, nullptr);
                    module_t mod;
                    mod.base = base;
                    mod.size = size;
                    mod.name = name_buf;
                    modules.push_back(mod);
                }
            }

            uint64_t next = 0;
            if (hypercall::read_guest_virtual_memory(&next, current_entry, target_cr3, sizeof(next)) != sizeof(next)) break;
            if (!next) break;
            current_entry = next;
        }

        return !modules.empty();
    }

    uint64_t get_pid(const std::string& name)
    {
        const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE)
            return 0;

        wchar_t wide_name[MAX_PATH]{};
        MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wide_name, MAX_PATH);

        PROCESSENTRY32W entry = {};
        entry.dwSize = sizeof(entry);

        uint64_t found = 0;
        for (BOOL ok = Process32FirstW(snapshot, &entry); ok; ok = Process32NextW(snapshot, &entry))
        {
            if (_wcsicmp(entry.szExeFile, wide_name) == 0)
            {
                found = entry.th32ProcessID;
                break;
            }
        }

        CloseHandle(snapshot);
        return found;
    }

    static bool snapshot_modules(uint64_t pid)
    {
        const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
                                                         static_cast<DWORD>(pid));
        if (snapshot == INVALID_HANDLE_VALUE)
            return false;

        MODULEENTRY32W entry = {};
        entry.dwSize = sizeof(entry);

        modules.clear();
        for (BOOL ok = Module32FirstW(snapshot, &entry); ok; ok = Module32NextW(snapshot, &entry))
        {
            module_t mod;
            mod.base = reinterpret_cast<uint64_t>(entry.modBaseAddr);
            mod.size = static_cast<uint64_t>(entry.modBaseSize);
            char name_buf[MAX_PATH]{};
            WideCharToMultiByte(CP_UTF8, 0, entry.szModule, -1, name_buf, sizeof(name_buf), nullptr, nullptr);
            mod.name = name_buf;
            modules.push_back(mod);
        }

        CloseHandle(snapshot);
        return !modules.empty();
    }

    bool initialize(void)
    {
        if (available)
            return true;

        own_cr3 = read_own_cr3();
        available = (own_cr3 != 0) && (find_mmpfn_database() != 0);
        return available;
    }

    bool bind(const std::string& process_name)
    {
        target_process_name = process_name;

        if (!initialize())
            return false;

        if (!init_eprocess_offsets())
            return false;

        target_pid = get_pid(process_name);
        if (!target_pid)
            return false;

        
        
        uint64_t main_base = 0;
        const uint64_t peb = find_peb(target_pid, &main_base);
        if (!main_base)
            return false;

        target_base = main_base;

        const uint64_t pfn_db = find_mmpfn_database();
        target_cr3 = bruteforce_cr3(target_base, pfn_db);
        if (!target_cr3)
            return false;

        
        if (!snapshot_modules_from_peb(peb))
            snapshot_modules(target_pid);

        attached = true;
        return true;
    }

    bool refresh_modules(void)
    {
        if (!attached)
            return hvre::bind(target_process_name);

        const uint64_t live_pid = get_pid(target_process_name);
        if (!live_pid)
        {
            attached = false;
            return false;
        }

        if (live_pid != target_pid)
        {
            
            attached = false;
            return hvre::bind(target_process_name);
        }

        
        const uint64_t pfn_db = find_mmpfn_database();
        const uint64_t fresh = bruteforce_cr3(target_base, pfn_db);
        if (fresh)
            target_cr3 = fresh;

        
        const uint64_t peb = find_peb(target_pid, nullptr);
        if (peb && snapshot_modules_from_peb(peb))
            return true;

        snapshot_modules(target_pid);
        return true;
    }

    module_t get_module(const std::string& name)
    {
        std::string query_lower = name;
        for (auto& c : query_lower)
            c = static_cast<char>(tolower(static_cast<unsigned char>(c)));

        for (const auto& mod : modules)
        {
            std::string stored_lower = mod.name;
            for (auto& c : stored_lower)
                c = static_cast<char>(tolower(static_cast<unsigned char>(c)));

            if (stored_lower == query_lower ||
                strstr(stored_lower.c_str(), query_lower.c_str()) ||
                strstr(query_lower.c_str(), stored_lower.c_str()))
            {
                return mod;
            }
        }

        return {};
    }

    bool read(void* dst, uint64_t src, uint64_t size)
    {
        if (!attached || !target_cr3)
            return false;
        return hypercall::read_guest_virtual_memory(dst, src, target_cr3, size) == size;
    }

    bool write(uint64_t dst, const void* src, uint64_t size)
    {
        if (!attached || !target_cr3)
            return false;
        return hypercall::write_guest_virtual_memory(src, dst, target_cr3, size) == size;
    }

    uint64_t translate(uint64_t va, uint64_t cr3)
    {
        return hypercall::translate_guest_virtual_address(va, cr3);
    }
} 