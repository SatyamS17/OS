#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"
#include "file_system.h"

extern void terminal_init_buffer(void);

extern int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t terminal_open(const uint8_t* filename);
extern int32_t terminal_close(int32_t fd);

extern func_pt_t make_stdin_fops(void);
extern func_pt_t make_stdout_fops(void);

#endif  /* _TERMINAL_H */
