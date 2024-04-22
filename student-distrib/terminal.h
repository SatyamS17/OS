#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"
#include "file_system.h"
#include "keyboard.h"

typedef struct terminal_state {
    keyboard_buffer_t kb_buffer;
    int cursor_x;
    int cursor_y;
} terminal_state_t;

extern void terminal_init(void);

extern void terminal_switch(int32_t idx);

extern int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t terminal_open(const uint8_t* filename);
extern int32_t terminal_close(int32_t fd);

extern func_pt_t make_stdin_fops(void);
extern func_pt_t make_stdout_fops(void);

#endif  /* _TERMINAL_H */
