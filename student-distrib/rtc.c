#include "rtc.h"

#include "i8259.h"
#include "lib.h"

#define RTC_IRQ 8
#define RTC_IO_PORT 0x70
#define CMOS_IO_PORT 0x71
#define RTC_REG_A 0x8A
#define RTC_REG_B 0x8B
#define RTC_REG_C 0x8C

/* extern void rtc_init(void)
 * Inputs: void
 * Return Value: N/A
 * Function: initializes IRQ number for RTC (IRQ8)*/
void rtc_init(void) {
    unsigned char prev;

    outb(RTC_REG_B, RTC_IO_PORT);
    prev = inb(CMOS_IO_PORT);
    outb(RTC_REG_B, RTC_IO_PORT);
    outb(prev | 0x40, CMOS_IO_PORT);

    uint32_t rate = 6;
    outb(RTC_REG_A, RTC_IO_PORT);
    prev = inb(CMOS_IO_PORT);
    outb(RTC_REG_A, RTC_IO_PORT);
    outb((prev & 0xF0) | rate, CMOS_IO_PORT);

    enable_irq(RTC_IRQ);
}

/* void rtc_handler(void)
 * Inputs: void
 * Return Value: N/A
 * Function: executes interrupts from RTC*/
void rtc_handler_base(void) {
    outb(RTC_REG_C, RTC_IO_PORT);
    inb(CMOS_IO_PORT);
    
    test_interrupts();

    send_eoi(RTC_IRQ);
}
