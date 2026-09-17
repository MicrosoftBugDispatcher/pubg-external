.code

PUBLIC launch_raw_hypercall

; uint64_t launch_raw_hypercall(hypercall_info_t rcx, uint64_t rdx, uint64_t r8, uint64_t r9)
launch_raw_hypercall proc
    cpuid
    ret
launch_raw_hypercall endp

end
