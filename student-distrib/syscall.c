#include "syscall.h"

#include "file_system.h"
#include "lib.h"
#include "paging.h"
#include "rtc.h"
#include "terminal.h"

pcb_t* curr_pcb = NULL;

static uint32_t pids[MAXPIDS] = {0};

int32_t halt(uint8_t status) {
    printf("halt called\n");
    return 0;
}

int32_t execute(const uint8_t* command) {
    int i;

    // -------- Parse args -------- //
    

    // Get file name from command (text until first space)
    uint8_t file_name[FILENAME_SIZE];
    memcpy((void*) file_name, (const void*)command, sizeof(uint8_t) * FILENAME_SIZE);

    if (curr_pcb != NULL) {
        file_name[0] = 't';
        file_name[1] = 'e';
        file_name[2] = 's';
        file_name[3] = 't';
        file_name[4] = 'p';
        file_name[5] = 'r';
        file_name[6] = 'i';
        file_name[7] = 'n';
        file_name[8] = 't';
        file_name[9] = '\0';
    }
    for (i = 9; i < FILENAME_SIZE; i++) {
        file_name[i] = '\0';
    }

    // for (i = 0; i < FILENAME_SIZE - 1; i++) {
    //     if (command[i] == ' ') {
    //         i++;
    //         break;
    //     }
    //     file_name[i] = command[i];
    // }

    // file_name[i] = '\0';  // Null terminate
    
    // -------- Check File Validity -------- //
    dentry_t dentry; 
    
    //file doesn't exist check 
    if (read_dentry_by_name(file_name, &dentry) == -1) {
        printf("Error: Command `%s` doesn't exist\n", file_name);
        return -1;
    }

    //check if we can read 4 bytes  
    uint8_t elf_buffer[ELF_SIZE];
    
    if (read_data(dentry.inode, 0, elf_buffer, ELF_SIZE) == -1) {
        printf("Error: Unable to read first 4 bytes\n");
        return -1;
    }

    //check if executable  
    if (elf_buffer[0] != ELF_MN_1 || 
        elf_buffer[1] != ELF_MN_2 ||
        elf_buffer[2] != ELF_MN_3 || 
        elf_buffer[3] != ELF_MN_4) { 
        printf("Error: `%s` is not an executable\n", file_name);
        return -1;
    }

    // -------- Find PID -------- /
    int32_t old_pid;
    if (curr_pcb == NULL) {
        old_pid = -1;
    } else {
        old_pid = curr_pcb->pid;
    }
    
    int pid = -1;
    for (i = 0; i < MAXPIDS; i++){
        // select a valid PID
        if (pids[i] == 0){
            pids[i] = 1;
            pid = i;
            break;
        }
    }

    if (pid == -1) {
        printf("Error: All PIDs used\n");
        return -1;
    }

    curr_pcb = (pcb_t*) (KERNEL_END - (EIGHTKB_BITS * (pid + 1)));
    curr_pcb->parent_pid = old_pid;
    curr_pcb->pid = pid;
    curr_pcb->ebp = (USER_ADDRESS + FOURMB_BITS);
    curr_pcb->esp = (USER_ADDRESS + FOURMB_BITS);

    // -------- Setup Paging -------- //

    // make virtual mem map to right physical address
    uint32_t physical_address = KERNEL_END + (curr_pcb->pid * FOURMB_BITS);
    page_dir[USER_INDEX].page_table_address = physical_address / FOURKB_BITS;   // UNSURE ABOUT THIS *TEST*
    flush_tlb();
        
    // -------- load file into memory -------- //

    // know it's an executable now copy to physical address
    uint8_t* program_location = (uint8_t*) (USER_ADDRESS + USER_OFFSET);
    uint32_t file_size = ((inodes_t *)((uint8_t *)file_system + ((dentry.inode + 1) * (BLOCK_SIZE))))->length;
    read_data(dentry.inode, 0, program_location, file_size);

    uint32_t eip = *((uint32_t*)(program_location + 24));

    // -------- create pcb and open fds -------- //

    // Clear all FDs
    for (i = 0; i < MAX_OPEN_FILES; i++) {
        curr_pcb->fds[i].functions.open = NULL;
        curr_pcb->fds[i].functions.close = NULL;
        curr_pcb->fds[i].functions.read = NULL;
        curr_pcb->fds[i].functions.write = NULL;
        curr_pcb->fds[i].inode = 0;
        curr_pcb->fds[i].pos = 0;
        curr_pcb->fds[i].flags = FD_AVAIL;   
    }

    // Set FD 0 to stdin
    curr_pcb->fds[0].functions = make_stdin_fops();
    curr_pcb->fds[0].flags = FD_USED;

    // Set FD 1 to stdin
    curr_pcb->fds[1].functions = make_stdout_fops();
    curr_pcb->fds[1].flags = FD_USED;

    // PID and parent PID already set

    // -------- Prepare For Context Switch -------- //

    // modify esp0 and ss0 in TSS
    uint32_t process_kernel_address = KERNEL_END - (curr_pcb->pid * EIGHTKB_BITS);
    tss.ss0 = KERNEL_DS;
    tss.esp0 = process_kernel_address;

    // get esp to push to stack
    asm volatile("mov %%esp, %0" : "=r" (curr_pcb->esp));
    asm volatile("mov %%ebp, %0" : "=r" (curr_pcb->ebp));

    // push IRET context on the the correct order
    asm volatile("    \n\
        pushl %0      \n\
        pushl %1      \n\
        pushfl        \n\
        pushl %2      \n\
        pushl %3      \n\
        iret          \n\
        "
        :
        : "r" (USER_DS), "r" (USER_ADDRESS + FOURMB_BITS), "r" (USER_CS), "r" (eip)
        : "memory"
    );

    return 0;
}

int32_t read(int32_t fd, void* buf, int32_t nbytes) {
    if(fd < 0 || fd > MAX_OPEN_FILES
       || buf == NULL || nbytes < 0
       || curr_pcb->fds[fd].flags == FD_AVAIL){
        return -1;
    }
    
    int32_t bytes = curr_pcb->fds[fd].functions.read(fd,buf,nbytes);
    curr_pcb->fds[fd].pos += bytes;
    return bytes;
}

int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
    if(fd < 0 || fd > MAX_OPEN_FILES
       || buf == NULL || nbytes < 0
       || curr_pcb->fds[fd].flags == FD_AVAIL){
        return -1;
    }    
    return curr_pcb->fds[fd].functions.write(fd,buf,nbytes);
}

int32_t open(const uint8_t* filename) {
    dentry_t dentry;
    if (read_dentry_by_name(filename, &dentry) != 0) {
        return -1;
    }

    // Find open file descriptor index
    int fd_idx;
    for (fd_idx = 0; fd_idx < MAX_OPEN_FILES; fd_idx++) {
        if (curr_pcb->fds[fd_idx].flags == FD_AVAIL) {
            break;
        }
    }

    // No open file descriptors
    if (fd_idx == MAX_OPEN_FILES) {
        return -1;
    }

    curr_pcb->fds[fd_idx].inode = dentry.inode;
    curr_pcb->fds[fd_idx].pos = 0;
    curr_pcb->fds[fd_idx].flags = FD_USED;

    switch (dentry.file_type) {
    case 0:  // RTC
        curr_pcb->fds[fd_idx].functions = make_rtc_fops();    
        break;
    case 1:  // directory
        curr_pcb->fds[fd_idx].functions = make_dir_fops();
        break;
    case 2:  // regular file
        curr_pcb->fds[fd_idx].functions = make_file_fops();
        break;
    }

    return -1;
} 

//close the fd and set it to available
int32_t close(int32_t fd) {
    if(fd < 0 || fd > MAX_OPEN_FILES || curr_pcb->fds[fd].flags == FD_AVAIL){
        return -1;
    }
    curr_pcb->fds[fd].functions.close(fd);
    curr_pcb->fds[fd].flags = FD_AVAIL;

    return 0;
}

int32_t getargs(uint8_t* buf, int32_t nbytes) {
    printf("getargs called\n");
    return 0;
}

int32_t vidmap(uint8_t** screen_start) {
    printf("vidmap called\n");
    return 0;
}

int32_t set_handler(int32_t signum, void* handler_address) {
    printf("set_handler called\n");
    return 0;
}

int32_t sigreturn(void) {
    printf("sigreturn called\n");
    return 0;
}
