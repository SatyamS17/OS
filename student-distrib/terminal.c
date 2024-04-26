#include "terminal.h"

#include "keyboard.h"
#include "lib.h"
#include "paging.h"
#include "syscall.h"
#include "x86_desc.h"
#include "scheduling.h"

static terminal_state_t terminals[NUM_TERMINALS];

uint8_t screen_terminal_idx = 0;

terminal_state_t* terminal_get_state(uint8_t index) {
    return &terminals[index];
}

/* void terminal_init(void)
 * Inputs: None
 * Return Value: int32_t 0 or -1 for success or fail
 * Function: initializes terminal buffer
 */
void terminal_init(void) {
    int i;
    for (i = 0; i < NUM_TERMINALS; i++) {
        memset(terminals[i].kb_buffer.buf, 0, BUFFER_SIZE);
        terminals[i].kb_buffer.idx = 0;
        terminals[i].kb_buffer.data_available = 0;
        terminals[i].cursor_x = 0;
        terminals[i].cursor_y = 0;
    }

    set_screen_xy(&terminals[0].cursor_x, &terminals[0].cursor_y);
    keyboard_set_buffer(&terminals[0].kb_buffer);
}

void terminal_switch(uint8_t idx) {
    uint8_t old_terminal_idx = screen_terminal_idx;
    uint8_t new_terminal_idx = idx;

    if (new_terminal_idx >= NUM_TERMINALS) {
        return;
    }
    
    cli();

    // Copy video memmory to old terminal's video memory and copy new terminal's video memory to video memory.
    uint32_t prev_base_address = page_table[VID_MEM_INDEX].base_address;
    page_table[VID_MEM_INDEX].base_address = VID_MEM_INDEX;
    uservid_page_table[VID_MEM_INDEX].base_address = VID_MEM_INDEX;
    flush_tlb();

    memcpy((void*) (VID_MEM + ((old_terminal_idx + 1) * FOURKB_BITS)), (void*) VID_MEM, FOURKB_BITS);
    memcpy((void*) VID_MEM, (void*) (VID_MEM + ((new_terminal_idx + 1) * FOURKB_BITS)), FOURKB_BITS);

    page_table[VID_MEM_INDEX].base_address = prev_base_address;
    uservid_page_table[VID_MEM_INDEX].base_address = prev_base_address;
    flush_tlb();

    // Set keyboard buffer and cursor to new terminal
    terminal_state_t *new_terminal_state = terminal_get_state(new_terminal_idx);
    keyboard_set_buffer(&new_terminal_state->kb_buffer);
    set_screen_xy(&new_terminal_state->cursor_x, &new_terminal_state->cursor_y);
    set_cursor(new_terminal_state->cursor_x, new_terminal_state->cursor_y);

    // Current process's terminal was on the screen but we're switching away
    if (scheduler_terminal_idx == old_terminal_idx) {
        page_table[VID_MEM_INDEX].base_address = VID_MEM_INDEX + (old_terminal_idx + 1);
        uservid_page_table[VID_MEM_INDEX].base_address = VID_MEM_INDEX + (old_terminal_idx + 1);
        flush_tlb();
    }

    // Current process's terminal was not on the screen but we're switching onto it
    if (scheduler_terminal_idx == new_terminal_idx) {
        page_table[VID_MEM_INDEX].base_address = VID_MEM_INDEX;
        uservid_page_table[VID_MEM_INDEX].base_address = VID_MEM_INDEX;

        flush_tlb();
    }

    screen_terminal_idx = idx;

    sti();
}

/* int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: int32_t fd - file descriptor
           void* buf - buffer holding bytes
           int32_t nbytes - number of bytes able to be entered into buf
 * Return Value: int32_t of number of bytes written to buf
 * Function: keyboard presses and stores into buffer buf until enter is pressed
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes) {
    // Wait until enter key pressed.
    uint8_t idx = scheduler_terminal_idx;
    while (!terminals[idx].kb_buffer.data_available) { }

    unsigned long flags;
    cli_and_save(flags);

    char* buf_char = (char*) buf;

    int bytes_read;
    for (bytes_read = 0; bytes_read < nbytes && bytes_read < BUFFER_SIZE; bytes_read++) {
        if (terminals[idx].kb_buffer.buf[bytes_read] == '\n') {
            break;
        }
        buf_char[bytes_read] = terminals[idx].kb_buffer.buf[bytes_read];
    }

    buf_char[bytes_read++] = '\n';

    memset(terminals[idx].kb_buffer.buf, 0, BUFFER_SIZE);
    terminals[idx].kb_buffer.idx = 0;
    terminals[idx].kb_buffer.data_available = 0;

    restore_flags(flags);
    return bytes_read;
}

/* int32_t terminal_write(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: int32_t fd - file descriptor
           void* buf - buffer holding bytes
           int32_t nbytes - number of bytes able to be entered into buf
 * Return Value: int32_t of number of bytes written to video memory
 * Function: outputs data from buf to video memory
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes) {
    (void) fd;

    const char* buf_char = (const char*) buf;
    if (buf_char == NULL) {
        return -1;
    }

    int bytes_written = 0;
    int i;
    for (i = 0; i < nbytes; i++) {
        if (buf_char[i] != 0) {
            putc(buf_char[i]);
            bytes_written++;
        }
    }
    return bytes_written;
}

/* int32_t terminal_open(const uint8_t* filename)
 * Inputs: const uint8_t* filename - name of file
 * Return Value: int32_t 0 or -1 for success or fail
 * Function: initializes terminal
 */
int32_t terminal_open(const uint8_t* filename) {
    return -1;
}

/* int32_t terminal_close(const uint8_t* filename)
 * Inputs: int32_t fd - file descriptor
 * Return Value: int32_t 0 or -1 for success or fail
 * Function: ends terminal
 */
int32_t terminal_close(int32_t fd) {
    return -1;
}

func_pt_t make_stdin_fops(void) {
    func_pt_t f;
    f.open = terminal_open;
    f.close = terminal_close;
    f.read = terminal_read;
    f.write = NULL;
    return f;
}

func_pt_t make_stdout_fops(void) {
    func_pt_t f;
    f.open = terminal_open;
    f.close = terminal_close;
    f.read = NULL;
    f.write = terminal_write;
    return f;
}
