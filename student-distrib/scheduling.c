#include "scheduling.h"

#include "terminal.h"
#include "paging.h"
#include "lib.h"
#include "syscall.h"

int current_tp_index = 0;

void scheduler() {
    // init terminal if needed
    
    if (get_curr_pcb() == NULL) {
        terminal_state_t *current_terminal_state = terminal_get_state(current_tp_index);
        keyboard_set_buffer(&current_terminal_state->kb_buffer);
        set_screen_xy(&current_terminal_state->cursor_x, &current_terminal_state->cursor_y);

        if (current_tp_index == terminal_get_curr_idx()) {
            page_table[VID_MEM_INDEX].base_address = VID_MEM_INDEX;    
            set_cursor(current_terminal_state->cursor_x, current_terminal_state->cursor_y);
        } else {
            page_table[VID_MEM_INDEX].base_address = VID_MEM_INDEX + (current_tp_index + 1);
        }
        
        execute((uint8_t * )"shell");
    }

    register uint32_t saved_ebp asm("ebp");    
    get_curr_pcb()->saved_ebp = saved_ebp;

    //look at next process in "queue"
    current_tp_index = (current_tp_index + 1) % 3;

    if (get_curr_pcb() == NULL) {
        return; 
    }
    
    // update page table
    page_dir[USER_INDEX].page_table_address = (KERNEL_END + (get_curr_pcb()->pid * FOURMB_BITS)) >> ADDRESS_SHIFT;

    // switch the vid memory being written
    terminal_state_t *current_terminal_state = terminal_get_state(current_tp_index);
    keyboard_set_buffer(&current_terminal_state->kb_buffer);
    set_screen_xy(&current_terminal_state->cursor_x, &current_terminal_state->cursor_y);
    if (current_tp_index == terminal_get_curr_idx()) {
        page_table[VID_MEM_INDEX].base_address = VID_MEM_INDEX;
        set_cursor(current_terminal_state->cursor_x, current_terminal_state->cursor_y);
    } else {
        page_table[VID_MEM_INDEX].base_address = VID_MEM_INDEX + (current_tp_index + 1);
    }

    
    flush_tlb();

    //save esp0 in the TSS
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_END - (get_curr_pcb()->pid * EIGHTKB_BITS);

    // some ASM code to restore ebp
    asm volatile("          \n\
        movl %0, %%ebp      \n\
        leave               \n\
        ret                 \n\
        "
        :
        : "r"(get_curr_pcb()->saved_ebp)
        : "ebp"
    );
    
}
