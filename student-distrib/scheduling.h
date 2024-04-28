#ifndef _SCHEDULING_H
#define _SCHEDULING_H

#include "syscall.h"
#include "terminal.h"

/* Index of the terminal the process thescheduler is currently running is in. */
extern uint8_t scheduler_terminal_idx;

/* Context switches between terminals in the background when called by the PIT. */
void scheduler();

#endif
