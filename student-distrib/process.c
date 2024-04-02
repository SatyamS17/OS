#include "process.h"
#include "x86_desc.h"
#include "terminal.h"

// Process Init (in mem) (assume pid is valid)
pcb_t * process_init(uint32_t pid) {
    
    // add PCB to kernel stack based on PID
        // get address to insert PCB in 8MB - (8KB * PID)
    uint32_t process_kernel_address = KERNEL_END - (pid * EIGHTKB_SIZE);
    pcb_t * process = (pcb_t *)process_kernel_address;

    process->pid = pid;

    // // create page for user space based on PID
    //     // get address to insert page in PA = 8MB + (PID * 4MB)
    // uint32_t * process_physical_address = KERNEL_END + (pid * FOURMB_SIZE);

    // // since 0 and 1 are used for kernel and vid mem? --------WORK ON THIS PART!!--------------------------
    // uint32_t page_dir_index = 2 + pid;
    // uint32_t page_table_index = ((int)process_physical_address) / FOURKB_BITS;

    // init pcb (create pid, file destriptor array) ---- CREATE DEFAULT FD ENTRIES!!-------------
    
    
    process->fds[0].functions = &terminal_functions;
    process->fds[1].functions = &terminal_functions;



    // -1 to suggest not init yet
    process->parent_pid = -1;

    return process;
}   

// save the state (general registers) into the current process's tss
void save_process_state(pcb_t * process, uint32_t esp, uint32_t ebp) {
    process->ebp = ebp;
    process->esp = esp;
}

// restore the process's tss into the OS's tss
void restore_process_state(pcb_t * process, tss_t * tss) {
    tss->ebp = process->ebp;
    tss->esp = process->esp;
}

void process_switch() {
    return;
}


