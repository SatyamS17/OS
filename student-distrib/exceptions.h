#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

/* Handler for interrupt 0 - Divide Error Exception. */
extern void divide_error_exception_handler(void);
extern void divide_error_exception_handler_base(void);

/* Handler for interrupt 1 - Debug Exception. */
extern void debug_exception_handler(void);
extern void debug_exception_handler_base(void);

/* Handler for interrupt 2 - NMI Interrupt. */
extern void nmi_interrupt_handler(void);
extern void nmi_interrupt_handler_base(void);

/* Handler for interrupt 3 - Breakpoint Exception. */
extern void breakpoint_exception_handler(void);
extern void breakpoint_exception_handler_base(void);

/* Handler for interrupt 4 - Overflow Exception. */
extern void overflow_exception_handler(void);
extern void overflow_exception_handler_base(void);

/* Handler for interrupt 5 - BOUND Range Exceeded Exception. */
extern void bound_range_exceeded_exception_handler(void);
extern void bound_range_exceeded_exception_handler_base(void);

/* Handler for interrupt 6 - Invalid Opcode Exception. */
extern void invalid_opcode_exception_handler(void);
extern void invalid_opcode_exception_handler_base(void);

/* Handler for interrupt 7 - Device Not Available Exception. */
extern void device_not_available_exception_handler(void);
extern void device_not_available_exception_handler_base(void);

/* Handler for interrupt 8 - Double Fault Exception. */
extern void double_fault_exception_handler(void);
extern void double_fault_exception_handler_base(void);

/* Handler for interrupt 9 - Coprocessor Segment Overrun. */
extern void coprocessor_segment_overrun_handler(void);
extern void coprocessor_segment_overrun_handler_base(void);

/* Handler for interrupt 10 - Invalid TSS Exception. */
extern void invalid_tss_exception_handler(void);
extern void invalid_tss_exception_handler_base(void);

/* Handler for interrupt 11 - Segment Not Present. */
extern void segment_not_present_handler(void);
extern void segment_not_present_handler_base(void);

/* Handler for interrupt 12 - Stack Fault Exception. */
extern void stack_fault_exception_handler(void);
extern void stack_fault_exception_handler_base(void);

/* Handler for interrupt 13 - General Protection Exception. */
extern void general_protection_exception_handler(void);
extern void general_protection_exception_handler_base(void);

/* Handler for interrupt 14 - Page-Fault Exception. */
extern void page_fault_exception_handler(void);
extern void page_fault_exception_handler_base(void);

/* Handler for interrupt 16 - x87 FPU Floating-Point Error. */
extern void x87_fpu_floating_point_error_handler(void);
extern void x87_fpu_floating_point_error_handler_base(void);

/* Handler for interrupt 17 - Alignment Check Exception. */
extern void alignment_check_exception_handler(void);
extern void alignment_check_exception_handler_base(void);

/* Handler for interrupt 18 - Machine-Check Exception. */
extern void machine_check_exception_handler(void);
extern void machine_check_exception_handler_base(void);

/* Handler for interrupt 19 - SIMD Floating-Point Exception. */
extern void simd_floating_point_exception_handler(void);
extern void simd_floating_point_exception_handler_base(void);

#endif /* _EXCEPTIONS_H */
