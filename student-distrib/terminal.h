#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "file_system.h"
#include "keyboard.h"
#include "syscall.h"
#include "types.h"

#define NUM_TERMINALS 3

/* State specific to a terminal. */
typedef struct terminal_state {
    /* Buffer containing keyboard text. */
    keyboard_buffer_t kb_buffer;

    /* Coordinate of cursor on screen. */
    int cursor_x;
    int cursor_y;

    /* RTC flag/counter */
    volatile uint8_t rtc_interrupt_flag;
    volatile uint32_t rtc_interrupt_counter;

    /* Pointer to this terminal's curernt process's PCB. */
    pcb_t *curr_pcb;
} terminal_state_t;

/* Index of the terminal currently shown on screen. */
extern uint8_t screen_terminal_idx;

extern terminal_state_t *terminal_get_state(uint8_t idx);

extern void terminal_init(void);

extern void terminal_switch(uint8_t idx);

extern int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes);
extern int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes);
extern int32_t terminal_open(const uint8_t *filename);
extern int32_t terminal_close(int32_t fd);

extern func_pt_t make_stdin_fops(void);
extern func_pt_t make_stdout_fops(void);

#endif /* _TERMINAL_H */
