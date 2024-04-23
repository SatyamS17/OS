#include "pit.h"

#include "i8259.h"
#include "lib.h"

/*
 * void pit_init(void)
 * Description: Initializes the PIT to 100 Hz
 * Inputs: None
 * Outputs: None
 */ 

//https://forum.osdev.org/viewtopic.php?f=1&t=40829
void pit_init(void){
    outb(PIT_MODE, PIT_CMD); 
    outb((uint8_t)(PIT_RATE & 0xFF), PIT_DATA);
    outb((uint8_t)(PIT_RATE >> 8) & 0xFF, PIT_DATA);

    enable_irq(PIT_IRQ);
}  

/*
 * void pit_handler(void)
 * Description: handle pit interrupts and context switch
 * Inputs: None
 * Outputs: None
 */  

void pit_handler(void){ 
    send_eoi(PIT_IRQ); 
    scheduler();
}

