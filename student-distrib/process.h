#ifndef _PROCESS_H
#define _PROCESS_H

#include "file_system.h"

#define KERNEL_END      0x800000
#define EIGHTKB_SIZE    (FOURKB_BITS * 2)

#define FOURMB_SIZE     0x400000

#define MAX_OPEN_FILES  8

pcb_t * process_init(uint32_t pid);

/* function pointer struct */
typedef struct func_pt {
    uint32_t (*open)(const uint8_t * filename);
    uint32_t (*close)(uint32_t fd);
    uint32_t (*read)(uint32_t fd, void * buf, uint32_t nbytes);
    uint32_t (*write)(uint32_t fd, const void* buf, uint32_t nbytes);
} func_pt_t;

/* PCB Struct*/
typedef struct pcb {
    fd_t fds[MAX_OPEN_FILES];

    int32_t pid;     // process ID
    int32_t parent_pid; // used for context switching 

    // information about current process
    uint32_t esp;
    uint32_t ebp;
} pcb_t;

#endif