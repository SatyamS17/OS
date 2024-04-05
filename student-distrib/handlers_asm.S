#define ASM 1

#define link_asm(name, base)  \
.globl name                   ;\
name:                         ;\
    pushal                    ;\
    pushfl                    ;\
    call base                 ;\
    popfl                     ;\
    popal                     ;\
    iret                      ;\

link_asm(divide_error_exception_handler, divide_error_exception_handler_base)
link_asm(debug_exception_handler, debug_exception_handler_base)
link_asm(nmi_interrupt_handler, nmi_interrupt_handler_base)
link_asm(breakpoint_exception_handler, breakpoint_exception_handler_base)
link_asm(overflow_exception_handler, overflow_exception_handler_base)
link_asm(bound_range_exceeded_exception_handler, bound_range_exceeded_exception_handler_base)
link_asm(invalid_opcode_exception_handler, invalid_opcode_exception_handler_base)
link_asm(device_not_available_exception_handler, device_not_available_exception_handler_base)
link_asm(double_fault_exception_handler, double_fault_exception_handler_base)
link_asm(coprocessor_segment_overrun_handler, coprocessor_segment_overrun_handler_base)
link_asm(invalid_tss_exception_handler, invalid_tss_exception_handler_base)
link_asm(segment_not_present_handler, segment_not_present_handler_base)
link_asm(stack_fault_exception_handler, stack_fault_exception_handler_base)
link_asm(general_protection_exception_handler, general_protection_exception_handler_base)
link_asm(page_fault_exception_handler, page_fault_exception_handler_base)
link_asm(x87_fpu_floating_point_error_handler, x87_fpu_floating_point_error_handler_base)
link_asm(alignment_check_exception_handler, alignment_check_exception_handler_base)
link_asm(machine_check_exception_handler, machine_check_exception_handler_base)
link_asm(simd_floating_point_exception_handler, simd_floating_point_exception_handler_base)

link_asm(keyboard_handler, keyboard_handler_base)
link_asm(rtc_handler, rtc_handler_base)
