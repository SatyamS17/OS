#include "exceptions.h"

#include "lib.h"
#include "syscall.h"

#define EXCEPTION_HANDLER(name, msg)               \
void name(void) {                                  \
    clear();                                       \
    printf(":( encountered an error: " msg "\n");  \
    get_scheduler_pcb()->exception_occured = 1;    \
    halt(0);                                       \
}

EXCEPTION_HANDLER(divide_error_exception_handler_base, "divide error exception")
EXCEPTION_HANDLER(debug_exception_handler_base, "debug exception")
EXCEPTION_HANDLER(nmi_interrupt_handler_base, "nmi interrupt")
EXCEPTION_HANDLER(breakpoint_exception_handler_base, "breakpoint exception")
EXCEPTION_HANDLER(overflow_exception_handler_base, "overflow exception")
EXCEPTION_HANDLER(bound_range_exceeded_exception_handler_base, "bound range exceeded exception")
EXCEPTION_HANDLER(invalid_opcode_exception_handler_base, "invalid opcode exception")
EXCEPTION_HANDLER(device_not_available_exception_handler_base, "device not available exception")
EXCEPTION_HANDLER(double_fault_exception_handler_base, "double fault exception")
EXCEPTION_HANDLER(coprocessor_segment_overrun_handler_base, "coprocessor segment overrun")
EXCEPTION_HANDLER(invalid_tss_exception_handler_base, "invalid tss exception")
EXCEPTION_HANDLER(segment_not_present_handler_base, "segment not present")
EXCEPTION_HANDLER(stack_fault_exception_handler_base, "stack fault exception")
EXCEPTION_HANDLER(general_protection_exception_handler_base, "general protection exception")
EXCEPTION_HANDLER(page_fault_exception_handler_base, "page fault exception")
EXCEPTION_HANDLER(x87_fpu_floating_point_error_handler_base, "x87 fpu floating point error")
EXCEPTION_HANDLER(alignment_check_exception_handler_base, "alignment check exception")
EXCEPTION_HANDLER(machine_check_exception_handler_base, "machine check exception")
EXCEPTION_HANDLER(simd_floating_point_exception_handler_base, "simd floating point exception")
