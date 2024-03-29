//https://play.typeracer.com?rt=19o9oge9sb
//https://humanbenchmark.com/tests/chimp
//https://arithmetic.zetamac.com/

#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "types.h"
#include "file_system.h"

/* vector entry of Intel syscall handler */
#define SYSCALL_HANDLER_VEC 0x80

extern void syscall_handler(void);

extern int32_t halt(uint8_t status);
extern int32_t execute(const uint8_t* command);
extern int32_t read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open(const uint8_t* filename);
extern int32_t close(int32_t fd);
extern int32_t getargs(uint8_t* buf, int32_t nbytes);
extern int32_t vidmap(uint8_t** screen_start);
extern int32_t set_handler(int32_t signum, void* handler_address);
extern int32_t sigreturn(void);

typedef struct context_regs {
    uint32_t eflags;
    uint32_t eip;
    uint32_t esp;
    uint32_t ebp;
    uint32_t ecx;
    uint32_t eax;
    uint32_t esi;
    uint32_t ebx;
    uint32_t edx;
    uint32_t edi;
} context_regs;
 
/* PCB Struct*/
typedef struct{
    fd_t fd_array[8]; // you can have 8 files in a process

    uint32_t pid;     // process ID
    uint32_t switch_pid; // used for context switching 

    // information about current process
    context_regs regs;
} pcb_t;

#endif  /* _SYSCALL_H */
