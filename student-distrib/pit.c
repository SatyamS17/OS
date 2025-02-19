#include "pit.h"

#include "i8259.h"
#include "lib.h"
#include "scheduling.h"

#define PIT_RATE 11932 // 1.19318 Mhz
#define PIT_DATA 0x40
#define PIT_CMD 0x43
#define PIT_MODE 0x37 // square wave
#define PIT_IRQ 0

/*
 * void pit_init(void)
 * Description: Initializes the PIT to 100 Hz
 * Inputs: None
 * Outputs: None
 */
// https://wiki.osdev.org/Programmable_Interval_Timer
void pit_init(void) {
    outb(PIT_MODE, PIT_CMD);
    outb((uint8_t)(PIT_RATE & 0xFF), PIT_DATA);        // send lower 8
    outb((uint8_t)((PIT_RATE >> 8) & 0xFF), PIT_DATA); // send upper 8

    enable_irq(PIT_IRQ);
}
/*
 * void pit_handler(void)
 * Description: handle pit interrupts and context switch
 * Inputs: None
 * Outputs: None
 */
void pit_handler_base(void) {
    send_eoi(PIT_IRQ);
    scheduler();
}
