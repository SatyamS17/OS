//https://play.typeracer.com?rt=19o9oge9sb
//https://humanbenchmark.com/tests/chimp
//https://arithmetic.zetamac.com/

#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "keyboard.h"
#include "file_system.h"
#include "types.h"
#include "x86_desc.h"

/* vector entry of Intel syscall handler */
#define SYSCALL_HANDLER_VEC 0x80

/* Max number of processes */
#define MAXPIDS 3

/* ELF magic constants */ 
#define ELF_MN_1 0x7f
#define ELF_MN_2 0x45 
#define ELF_MN_3 0x4c
#define ELF_MN_4 0x46

/* Offset for program image */
#define USER_OFFSET 0x48000

#define KERNEL_END      0x800000
#define EIGHTKB_BITS    (FOURKB_BITS * 2)
#define FOURMB_BITS     0x400000

/* Offset to find EIP in program image */
#define EIP_OFFSET  24

/* Max open file descriptors for a process */
#define MAX_OPEN_FILES  8

/* Use to get the base address */
#define ADDRESS_SHIFT       12

/* PCB Struct*/
typedef struct pcb {
    fd_t fds[MAX_OPEN_FILES];

    int32_t pid;        // process ID
    struct pcb* parent_pcb;  // used for context switching 

    // information about current process
    uint32_t saved_ebp;

    // Argument passed into shell
    uint8_t args[BUFFER_SIZE];

    // Boolean flag for if an exception has occured in this process or not.
    int32_t exception_occured;
} pcb_t;

/* Pointer to current PCB. */
extern pcb_t* curr_pcb;

/* Manually flush TLB cache. */
extern void flush_tlb(void);

/* General syscall handler */
extern void syscall_handler(void);

/* Syscalls */
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

#endif  /* _SYSCALL_H */
