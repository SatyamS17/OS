/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"

#include "lib.h"

/* Ports that each PIC sits on */
#define MASTER_8259_PORT 0x20
#define SLAVE_8259_PORT 0xA0

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1 0x11
#define ICW2_MASTER 0x20
#define ICW2_SLAVE 0x28
#define ICW3_MASTER 0x04
#define ICW3_SLAVE 0x02
#define ICW4 0x01

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI 0x60

/* Interrupt masks to determine which interrupts are enabled and disabled */
static uint8_t master_mask; /* IRQs 0-7  */
static uint8_t slave_mask;  /* IRQs 8-15 */

/* void i8259_init(void)
 * Inputs: void
 * Return Value: N/A
 * Function: initializes PIC for slave and master*/
void i8259_init(void) {
    /*mask out all interrupts*/
    master_mask = 0xFF;
    slave_mask = 0xFF;
    outb(master_mask, MASTER_8259_PORT + 1); /*+1 to access data*/
    outb(slave_mask, SLAVE_8259_PORT + 1);   /*+1 to access data*/

    /*initialize the PIC*/
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_PORT + 1);
    outb(ICW3_MASTER, MASTER_8259_PORT + 1);
    outb(ICW4, MASTER_8259_PORT + 1);

    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);
    outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);
    outb(ICW4, SLAVE_8259_PORT + 1);

    enable_irq(2); /*slave on irq2 of master*/
}

/* void enable_irq(uint32_t irq_num)
 * Inputs: uint32_t irq_num
 * Return Value: N/A
 * Function: Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    if (irq_num < 8) { /*if master PIC*/
        master_mask &= ~(1 << irq_num);
        outb(master_mask, MASTER_8259_PORT + 1);
    } else if (irq_num > 15) { /*15 is max num for interrupts*/
        return;
    } else {
        slave_mask &= ~(1 << (irq_num - 8));
        outb(slave_mask, SLAVE_8259_PORT + 1);
    }
}

/* void disable_irq(uint32_t irq_num)
 * Inputs: uint32_t irq_num
 * Return Value: N/A
 * Function: Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    if (irq_num < 8) { /*if master PIC*/
        master_mask |= (1 << irq_num);
        outb(master_mask, MASTER_8259_PORT + 1);
    } else if (irq_num > 15) { /*15 is max num for interrupts*/
        return;
    } else {
        slave_mask |= (1 << (irq_num - 8));
        outb(slave_mask, SLAVE_8259_PORT + 1);
    }
}

/* void send_eoi(uint32_t irq_num)
 * Inputs: uint32_t irq_num
 * Return Value: N/A
 * Function: Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    if (irq_num < 8) { /*if master PIC*/
        outb((EOI | irq_num), MASTER_8259_PORT);
    } else if (irq_num > 15) { /*15 is max num for interrupts*/
        return;
    } else {
        outb((EOI | (irq_num - 8)), SLAVE_8259_PORT);
        outb(EOI | 2, MASTER_8259_PORT); /*also mask master port that holds slave*/
    }
}
