#include "exceptions.h"

#include "lib.h"

#define EXCEPTION_HANDLER(name)               \
void name(void) {                             \
    printf("Exception caught: %s\n", #name);  \
    while (1) {}                              \
}

EXCEPTION_HANDLER(divide_error_exception_handler)
EXCEPTION_HANDLER(debug_exception_handler)
EXCEPTION_HANDLER(nmi_interrupt_handler)
EXCEPTION_HANDLER(breakpoint_exception_handler)
EXCEPTION_HANDLER(overflow_exception_handler)
EXCEPTION_HANDLER(bound_range_exceeded_exception_handler)
EXCEPTION_HANDLER(invalid_opcode_exception_handler)
EXCEPTION_HANDLER(device_not_available_exception_handler)
EXCEPTION_HANDLER(double_fault_exception_handler)
EXCEPTION_HANDLER(coprocessor_segment_overrun_handler)
EXCEPTION_HANDLER(invalid_tss_exception_handler)
EXCEPTION_HANDLER(segment_not_present_handler)
EXCEPTION_HANDLER(stack_fault_exception_handler)
EXCEPTION_HANDLER(general_protection_exception_handler)
EXCEPTION_HANDLER(page_fault_exception_handler)
EXCEPTION_HANDLER(x87_fpu_floating_point_error_handler)
EXCEPTION_HANDLER(alignment_check_exception_handler)
EXCEPTION_HANDLER(machine_check_exception_handler)
EXCEPTION_HANDLER(simd_floating_point_exception_handler)
