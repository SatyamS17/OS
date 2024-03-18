#include "keyboard.h"

#include "i8259.h"
#include "lib.h"

/* https://wiki.osdev.org/%228042%22_PS/2_Controller#Interrupts */
#define DATA_PORT 0x60
#define KEYBOARD_IRQ 1

#define DATA_TO_CHAR_SIZE 51


/* char data_to_char[DATA_TO_CHAR_SIZE]
holds conversions to ascii using data from keyboard */
char data_to_char[DATA_TO_CHAR_SIZE] = {
    0,
    0,
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    0,
    0,
    0,
    0,
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    0,
    0,
    0,
    0,
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    0,
    0,
    0,
    0,
    0,
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm'
};

/* extern void keyboard_init(void)
 * Inputs: void
 * Return Value: N/A
 * Function: initializes IRQ number for keyboard (IRQ1)*/
extern void keyboard_init(void) {
    enable_irq(KEYBOARD_IRQ);
}

/* void keyboard_handler(void)
 * Inputs: void
 * Return Value: N/A
 * Function: grabs keyboard input data and prints according ascii character*/
void keyboard_handler_base(void) {
    uint8_t data = inb(DATA_PORT);
    if (data < DATA_TO_CHAR_SIZE) {
        if (data_to_char[data]) {
            putc(data_to_char[data]);
        }
    }
    
    send_eoi(KEYBOARD_IRQ);
}
