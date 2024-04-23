#ifndef _SCHEDULING_H
#define _SCHEDULING_H 

#include "terminal.h"
#include "paging.h"
#include "syscall.c"

extern pcb_t * terminal_processes[NUM_TERMINALS];

void scheduler();

#endif

pcb_t * terminal_processes[NUM_TERMINALS];

int current_tp_index = 0;

void scheduler() {
    // save current ebp
    register uint32_t saved_ebp asm("ebp");    
    curr_pcb->saved_ebp = saved_ebp;

    //save esp0 in the TSS
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_END - (curr_pcb->pid * EIGHTKB_BITS);
    
    //look at next process in "queue"
    current_tp_index = (current_tp_index + 1) % 3;
    curr_pcb = terminal_processes[current_tp_index];

    // switch the vid memeory being written 
    page_table[VID_MEM_INDEX].base_address = VID_MEM_INDEX + (current_tp_index + 1);

    // update page table
    page_dir[USER_INDEX].page_table_address = (KERNEL_END + (curr_pcb->pid * FOURMB_BITS)) >> ADDRESS_SHIFT;

    flush_tlb();

    // some ASM code to restore ebp
    asm volatile("          \n\
        movl %0, %%ebp      \n\
        leave               \n\
        ret                 \n\
        "
        :
        : "r"(curr_pcb->saved_ebp)
    );
}