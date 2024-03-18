#ifndef _SYSCALL_H
#define _SYSCALL_H

/* vector entry of Intel syscall handler */
#define SYSCALL_HANDLER_VEC 0x80

/* Handler for system call interrupt. */
extern void syscall_handler(void);
extern void syscall_handler_base(void);

#endif  /* _SYSCALL_H */
