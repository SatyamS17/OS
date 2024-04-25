#include "keyboard.h"

#include "i8259.h"
#include "lib.h"
#include "paging.h"
#include "syscall.h"
#include "terminal.h"
#include "scheduling.h"

/* https://wiki.osdev.org/%228042%22_PS/2_Controller#Interrupts */
#define DATA_PORT 0x60
#define KEYBOARD_IRQ 1

#define BACKSPACE_PRESS 0x0E
#define BACKSPACE_RELEASE 0x8E

#define TAB_PRESS 0x0F
#define TAB_RELEASE 0x8F

#define CAPS_PRESS 0x3A
#define CAPS_RELEASE 0xBA

#define ENTER_PRESS 0x1C
#define ENTER_RELEASE 0x9C

#define LSHIFT_PRESS 0x2A
#define LSHIFT_RELEASE 0xAA

#define RSHIFT_PRESS 0x36
#define RSHIFT_RELEASE 0xB6

#define CTRL_PRESS 0x1D
#define CTRL_RELEASE 0x9D

#define ALT_PRESS 0x38
#define ALT_RELEASE 0xB8

#define F1_PRESS 0x3B
#define F2_PRESS 0x3C
#define F3_PRESS 0x3D
#define F4_PRESS 0x3E

/* How many spaces in a tab. */
#define TAB_NUM_SPACES 4

#define DATA_TO_CHAR_SIZE 60

/* Conversions from scancode to ASCII using data from keyboard */
char data_to_char[DATA_TO_CHAR_SIZE][2] = {
    {0, 0}, {0, 0},                      /* nothing */
    {'1', '!'}, {'2', '@'}, {'3', '#'},  /* row 1 */
    {'4', '$'}, {'5', '%'}, {'6', '^'},
    {'7', '&'}, {'8', '*'}, {'9', '('},
    {'0', ')'}, {'-', '_'}, {'=', '+'},

    {0, 0},                              /* backspace */
    {0, 0},                              /* tab */
    {'q', 'Q'}, {'w', 'W'}, {'e', 'E'},  /* row 2 */
    {'r', 'R'}, {'t', 'T'}, {'y', 'Y'},
    {'u', 'U'}, {'i', 'I'}, {'o', 'O'},
    {'p', 'P'}, {'[', '{'}, {']', '}'},

    {0, 0},                              /* enter */
    {0, 0},                              /* left control */
    {'a', 'A'}, {'s', 'S'}, {'d', 'D'},  /* row 3 */
    {'f', 'F'}, {'g', 'G'}, {'h', 'H'},
    {'j', 'J'}, {'k', 'K'}, {'l', 'L'},
    {';', ':'}, {'\'', '"'}, {'`', '~'},
    
    {0, 0},                              /* left shift */
    {'\\', '|'},
    {'z', 'Z'}, {'x', 'X'}, {'c', 'C'},  /* row 4 */
    {'v', 'V'}, {'b', 'B'}, {'n', 'N'},
    {'m', 'M'}, {',', '<'}, {'.', '>'},
    {'/', '?'},

    {0, 0},                              /* right shift */
    {0, 0},
    {0, 0},
    {' ', ' '},                          /* space */
    {0, 0},                              /* caps lock */
};

/* Flags for caps lshift rshift ctrl. */
static int capsbool, shiftbool, ctrlbool, altbool;

/* Pointer to buffer for characters entered. */
static keyboard_buffer_t* kb_buffer;
/* Number of characters since last enter key, used to keep track of backspaces. */
static int chars = 0;

/* void keyboard_set_buffer(keyboard_buffer_t* kb);
 * Inputs: keyboard_buffer_t* kb - pointer to keyboard buffer to set
 * Return Value: void
 * Function: configures the keyboard buffer to write to
 */
void keyboard_set_buffer(keyboard_buffer_t* kb) {
    kb_buffer = kb;
}

/* void keyboard_init(void)
 * Inputs: void
 * Return Value: N/A
 * Function: initializes IRQ number for keyboard (IRQ1)
 */
void keyboard_init(void) {
    enable_irq(KEYBOARD_IRQ);
}

/* void keyboard_handler(void)
 * Inputs: void
 * Return Value: N/A
 * Function: grabs keyboard input data and prints according ascii character
 */
void keyboard_handler_base(void) {
    uint8_t data = inb(DATA_PORT);

    //caps lshift rshift ctrl
    switch(data){
        case CAPS_PRESS:
            capsbool ^= 1;
            send_eoi(KEYBOARD_IRQ);
            return;
        case CAPS_RELEASE:
            send_eoi(KEYBOARD_IRQ);
            return;
        case RSHIFT_PRESS:
        case LSHIFT_PRESS:
            shiftbool = 1;
            send_eoi(KEYBOARD_IRQ);
            return;
        case RSHIFT_RELEASE:
        case LSHIFT_RELEASE:
            shiftbool = 0;
            send_eoi(KEYBOARD_IRQ);
            return;
        case CTRL_PRESS:
            ctrlbool = 1;
            send_eoi(KEYBOARD_IRQ);
            return;
        case CTRL_RELEASE:
            ctrlbool = 0;
            send_eoi(KEYBOARD_IRQ);
            return;
        case ALT_PRESS:
            altbool = 1;
            send_eoi(KEYBOARD_IRQ);
            return;
        case ALT_RELEASE:
            altbool = 0;
            send_eoi(KEYBOARD_IRQ);
            return;
    }
    
    if(altbool && data == F1_PRESS){
        terminal_switch(0);
        send_eoi(KEYBOARD_IRQ);
        return;
    }
    if(altbool && data == F2_PRESS){
        terminal_switch(1);
        send_eoi(KEYBOARD_IRQ);
        return;
    }
    if(altbool && data == F3_PRESS){
        terminal_switch(2);
        send_eoi(KEYBOARD_IRQ);
        return;
    }
    if(data == F4_PRESS) {
        send_eoi(KEYBOARD_IRQ);
        scheduler();
        return;
    }

    uint32_t prev_base_address = page_table[VID_MEM_INDEX].base_address;
    keyboard_buffer_t* prev_kb_buffer = kb_buffer;
    int* prev_screen_x = get_screen_x();
    int* prev_screen_y = get_screen_y();

    page_table[VID_MEM_INDEX].base_address = VID_MEM_INDEX;
    flush_tlb();

    terminal_state_t *current_terminal_state = terminal_get_state(terminal_get_curr_idx());
    keyboard_set_buffer(&current_terminal_state->kb_buffer);
    set_screen_xy(&current_terminal_state->cursor_x, &current_terminal_state->cursor_y);

    if (ctrlbool && data == 0x26) {
        clear();
    } else {
        if (kb_buffer == NULL) {
            if (data == BACKSPACE_PRESS && chars > 0) {
                putc(0x08); //this is ASCII for backspace value, rest is done in putc function
                chars--;
            } else if (data == ENTER_PRESS) {
                putc('\n');
                chars = 0;
            } else if (data == TAB_PRESS) {
                int i;
                for (i = 0; i < TAB_NUM_SPACES; i++) {
                    putc(' ');
                    chars++;
                }
            } else if (data < DATA_TO_CHAR_SIZE && data_to_char[data][capsbool ^ shiftbool]) {
                putc(data_to_char[data][capsbool ^ shiftbool]);
                chars++;
            }
        } else {
            if (data == BACKSPACE_PRESS && kb_buffer->idx > 0){
                putc(0x08); //this is ASCII for backspace value, rest is done in putc function
                kb_buffer->buf[--kb_buffer->idx] = 0;
                chars--;
            } else if (data == ENTER_PRESS && kb_buffer->idx <= (BUFFER_SIZE - 1)){
                putc('\n');
                kb_buffer->buf[kb_buffer->idx++] = '\n';
                kb_buffer->data_available = 1;
                chars = 0;
            } else if (data == TAB_PRESS && kb_buffer->idx < BUFFER_SIZE - TAB_NUM_SPACES) {
                int i;
                for (i = 0; i < TAB_NUM_SPACES; i++) {
                    putc(' ');
                    kb_buffer->buf[kb_buffer->idx++] = ' ';
                    chars++;
                }
            } else if (data < DATA_TO_CHAR_SIZE && data_to_char[data][capsbool ^ shiftbool]) {
                putc(data_to_char[data][capsbool ^ shiftbool]);
                chars++;
                if (kb_buffer->idx < (BUFFER_SIZE - 1)) {
                    kb_buffer->buf[kb_buffer->idx++] = data_to_char[data][capsbool ^ shiftbool];
                }
            }
        }
    }

    page_table[VID_MEM_INDEX].base_address = prev_base_address;
    flush_tlb();

    keyboard_set_buffer(prev_kb_buffer);
    set_screen_xy(prev_screen_x, prev_screen_y);

    send_eoi(KEYBOARD_IRQ);
}
