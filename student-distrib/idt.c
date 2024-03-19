#include "idt.h"

#include "exceptions.h"
#include "keyboard.h"
#include "rtc.h"
#include "syscall.h"
#include "x86_desc.h"

/* number of Intel defined excpetion vectors in IDT (IA-32 manual pg 5-23 to 5-58) */
#define NUM_EXCEPTIONS 20


/* extern void rtc_init(void)
 * Inputs: void
 * Return Value: N/A
 * Function: initializes IDT table*/
void init_idt() {
    /* Load IDT address into idt_desc_ptr. */
    lidt(idt_desc_ptr);

    int i;  /* looping index */

    /* Initialize all vectors in IDT. */
    for (i = 0; i < NUM_VEC; i++) {
        idt[i].seg_selector = KERNEL_CS;  /* kernel code segment                 */
        idt[i].size = 1;                  /* size=1 -> 32-bit gate               */
        idt[i].dpl = 0;                   /* low-privilege to prevent user calls */
        idt[i].present = 0;               /* default not present                 */
        idt[i].reserved0 = 0;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 1;
        idt[i].reserved4 = 0;
    }

    /* Enable interrupt vectors for exceptions. */
    for (i = 0; i < NUM_EXCEPTIONS; i++) {
        idt[i].present = 1;  /* mark entry present */
    }

    SET_IDT_ENTRY(idt[0], divide_error_exception_handler);
    SET_IDT_ENTRY(idt[1], debug_exception_handler);
    SET_IDT_ENTRY(idt[2], nmi_interrupt_handler);
    SET_IDT_ENTRY(idt[3], breakpoint_exception_handler);
    SET_IDT_ENTRY(idt[4], overflow_exception_handler);
    SET_IDT_ENTRY(idt[5], bound_range_exceeded_exception_handler);
    SET_IDT_ENTRY(idt[6], invalid_opcode_exception_handler);
    SET_IDT_ENTRY(idt[7], device_not_available_exception_handler);
    SET_IDT_ENTRY(idt[8], double_fault_exception_handler);
    SET_IDT_ENTRY(idt[9], coprocessor_segment_overrun_handler);
    SET_IDT_ENTRY(idt[10], invalid_tss_exception_handler);
    SET_IDT_ENTRY(idt[11], segment_not_present_handler);
    SET_IDT_ENTRY(idt[12], stack_fault_exception_handler);
    SET_IDT_ENTRY(idt[13], general_protection_exception_handler);
    SET_IDT_ENTRY(idt[14], page_fault_exception_handler);
    SET_IDT_ENTRY(idt[16], x87_fpu_floating_point_error_handler);
    SET_IDT_ENTRY(idt[17], alignment_check_exception_handler);
    SET_IDT_ENTRY(idt[18], machine_check_exception_handler);
    SET_IDT_ENTRY(idt[19], simd_floating_point_exception_handler);

    /* Enable interrupt vector for system calls. */
    idt[SYSCALL_HANDLER_VEC].present = 1;     /* mark entry present       */
    idt[SYSCALL_HANDLER_VEC].dpl = 3;         /* allow user calls         */
    SET_IDT_ENTRY(idt[SYSCALL_HANDLER_VEC], syscall_handler);

    /* Enable interrupt vector for keyboard interrupts. */
    idt[KEYBOARD_HANDLER_VEC].present = 1;    /* mark entry present       */
    idt[KEYBOARD_HANDLER_VEC].reserved3 = 0;  /* change to interrupt gate */
    SET_IDT_ENTRY(idt[KEYBOARD_HANDLER_VEC], keyboard_handler);

    /* Enable interrupt vector for RTC interrupts. */
    idt[RTC_HANDLER_VEC].present = 1;         /* mark entry present       */
    idt[RTC_HANDLER_VEC].reserved3 = 0;       /* change to interrupt gate */
    SET_IDT_ENTRY(idt[RTC_HANDLER_VEC], rtc_handler);
}
