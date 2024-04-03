//https://play.typeracer.com?rt=19o9oge9sb
//https://humanbenchmark.com/tests/chimp
//https://arithmetic.zetamac.com/

#ifndef _SYSCALL_H
#define _SYSCALL_H

#ifndef ASM

#include "types.h"
#include "file_system.h"
#include "x86_desc.h"

/* vector entry of Intel syscall handler */
#define SYSCALL_HANDLER_VEC 0x80
#define MAXPIDS 2 

/* ELF magic constants */ 
#define ELF_MN_1 0x7f
#define ELF_MN_2 0x45 
#define ELF_MN_3 0x4c
#define ELF_MN_4 0x46

#define USER_OFFSET 0x48000

extern void flush_tlb(void);

extern void iret_init(void);

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

uint32_t PIDS[MAXPIDS];
extern pcb_t * curr_pcb;

#endif  /* ASM */
#endif  /* _SYSCALL_H */