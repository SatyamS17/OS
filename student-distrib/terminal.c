#include "terminal.h"

#include "keyboard.h"
#include "lib.h"

/* Buffer for characters entered. */
static keyboard_buffer_t kb_buffer;

/* int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: int32_t fd - file descriptor
           void* buf - buffer holding bytes
           int32_t nbytes - number of bytes able to be entered into buf
 * Return Value: int32_t of number of bytes written to buf
 * Function: keyboard presses and stores into buffer buf until enter is pressed
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes) {
    // Wait until enter key pressed.
    while (!kb_buffer.data_available) { }

    unsigned long flags;
    cli_and_save(flags);

    char* buf_char = (char*) buf;

    int bytes_read;
    for (bytes_read = 0; bytes_read < nbytes && bytes_read < BUFFER_SIZE; bytes_read++) {
        if (kb_buffer.buf[bytes_read] == '\n') {
            break;
        }
        buf_char[bytes_read] = kb_buffer.buf[bytes_read];
    }

    buf_char[bytes_read++] = '\n';

    memset(kb_buffer.buf, 0, BUFFER_SIZE);
    kb_buffer.idx = 0;
    kb_buffer.data_available = 0;

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
    keyboard_set_buffer(&kb_buffer);
    return 0;
}

/* int32_t terminal_close(const uint8_t* filename)
 * Inputs: int32_t fd - file descriptor
 * Return Value: int32_t 0 or -1 for success or fail
 * Function: ends terminal
 */
int32_t terminal_close(int32_t fd) {
    keyboard_set_buffer(NULL);
    return 0;
}
