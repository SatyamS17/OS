#ifndef _SCHEDULING_H
#define _SCHEDULING_H 

#include "terminal.h"
#include "paging.h"
#include "syscall.h"

extern pcb_t * terminal_processes[NUM_TERMINALS];

void scheduler();

#endif
