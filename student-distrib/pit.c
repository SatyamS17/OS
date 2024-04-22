#include "pit.h"

#include "i8259.h"
#include "lib.h"

/*
 * pit_init
 * Description: Initializes the PIT to 100 Hz
 *  Inputs: None
 *  Outputs: None
 * Side Effects: None.
 */ 

//https://forum.osdev.org/viewtopic.php?f=1&t=40829
void pic_init(void){
    outb(PIT_MODE, PIT_CMD); 
    outb((uint8_t) PIT_RATE, PIT_DATA);
    outb((uint8_t) (PIT_RATE >> 8), PIT_DATA);

    enable_irq(PIT_IRQ); 
} 

