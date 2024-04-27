#ifndef _SCHEDULING_H
#define _SCHEDULING_H

#include "paging.h"
#include "syscall.h"
#include "terminal.h"

extern pcb_t *terminal_processes[NUM_TERMINALS];

extern uint8_t scheduler_terminal_idx;

void scheduler();

#endif
