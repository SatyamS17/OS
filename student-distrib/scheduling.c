#include "scheduling.h"

#include "terminal.h"
#include "paging.h"
#include "lib.h"
#include "syscall.h"

uint8_t scheduler_terminal_idx = 0;

void scheduler() {
    if (get_scheduler_pcb() == NULL) {
        terminal_state_t *current_terminal_state = terminal_get_state(scheduler_terminal_idx);
        keyboard_set_buffer(&current_terminal_state->kb_buffer);
        set_screen_xy(&current_terminal_state->cursor_x, &current_terminal_state->cursor_y);

        if (scheduler_terminal_idx == screen_terminal_idx) {
            page_table[VID_MEM_INDEX].base_address = VID_MEM_INDEX;
            set_cursor(current_terminal_state->cursor_x, current_terminal_state->cursor_y);
        } else {
            page_table[VID_MEM_INDEX].base_address = VID_MEM_INDEX + (scheduler_terminal_idx + 1);
        }

        flush_tlb();
        
        execute((uint8_t * ) "shell");
    }

    register uint32_t ebp asm("ebp");
    get_scheduler_pcb()->ebp_scheduler = ebp;

    // look at next process in "queue"
    scheduler_terminal_idx = (scheduler_terminal_idx + 1) % NUM_TERMINALS;

    if (get_scheduler_pcb() == NULL) {
        return;
    }
    
    // switch the vid memory being written
    terminal_state_t *current_terminal_state = terminal_get_state(scheduler_terminal_idx);
    keyboard_set_buffer(&current_terminal_state->kb_buffer);
    set_screen_xy(&current_terminal_state->cursor_x, &current_terminal_state->cursor_y);

    if (scheduler_terminal_idx == screen_terminal_idx) {
        page_table[VID_MEM_INDEX].base_address = VID_MEM_INDEX;
        set_cursor(current_terminal_state->cursor_x, current_terminal_state->cursor_y);
    } else {
        page_table[VID_MEM_INDEX].base_address = VID_MEM_INDEX + (scheduler_terminal_idx + 1);
    }

    flush_tlb();

    // update page table
    page_dir[USER_INDEX].page_table_address = (KERNEL_END + (get_scheduler_pcb()->pid * FOURMB_BITS)) >> ADDRESS_SHIFT;

    //save esp0 in the TSS
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_END - (get_scheduler_pcb()->pid * EIGHTKB_BITS);

    uint32_t saved_ebp = get_scheduler_pcb()->ebp_scheduler;

    // some ASM code to restore ebp
    asm volatile("          \n\
        movl %0, %%ebp      \n\
        leave               \n\
        ret                 \n\
        "
        :
        : "r"(saved_ebp)
        : "ebp"
    );
}
