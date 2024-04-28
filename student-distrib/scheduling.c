#include "scheduling.h"

#include "lib.h"
#include "paging.h"
#include "syscall.h"
#include "terminal.h"

uint8_t scheduler_terminal_idx = 0;

/*
 * scheduler()
 *   DESCRIPTION: Context switches between terminals in the background when called by the PIT
 *
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: void
 *   SIDE EFFECTS: Base case has it execute shell if terminal is empty otherwise context switches in a round robin way.
 */
void scheduler() {
    // if base case of empty terminal then call execute to initalize temrinal
    if (get_scheduler_pcb() == NULL) {
        // get the context of the terminal ready
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

        execute((uint8_t *)"shell");
    }

    // save terminal context before switching
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
        uservid_page_table[0].base_address = VID_MEM_INDEX;
        set_cursor(current_terminal_state->cursor_x, current_terminal_state->cursor_y);
    } else {
        page_table[VID_MEM_INDEX].base_address = VID_MEM_INDEX + (scheduler_terminal_idx + 1);
        uservid_page_table[0].base_address = VID_MEM_INDEX + (scheduler_terminal_idx + 1);
    }

    // update page table
    page_dir[USER_INDEX].page_table_address =
        (KERNEL_END + (get_scheduler_pcb()->pid * FOURMB_BITS)) >> ADDRESS_SHIFT;

    flush_tlb();

    // save esp0 in the TSS
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
                 : "ebp");
}
