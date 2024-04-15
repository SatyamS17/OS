#include "syscall.h"

#include "file_system.h"
#include "lib.h"
#include "paging.h"
#include "rtc.h"
#include "terminal.h"

/* Pointer to current PCB. */
pcb_t* curr_pcb = NULL;

/* Array of which PIDs are in use or not (1 or 0). */
static uint32_t pids[MAXPIDS];

/* int32_t halt(uint8_t status)
 *   DESCRIPTION: Ends process and returns context back to parent process.
 * 
 *   INPUTS: uint8_t status       : the current status passed by execute
 *   OUTPUTS: none
 *   RETURN VALUE: status
 *   SIDE EFFECTS: Returns to parent process
 */
int32_t halt(uint8_t status) {
    // If status = 256, an exception has occured and 256 should be returned from `execute`,
    // but.. status is a uint8_t so 256 wraps to 0.
    //
    // Instead, we manually store a flag of if an exception has occured.
    uint32_t status_32 = (uint32_t) status;
    if (curr_pcb->exception_occured) {
        status_32 = 256;
    }

    /* Check if trying to exit base shell */
    if (curr_pcb->parent_pcb == NULL) {
        // Clear base shell PID
        pids[0] = 0;
        curr_pcb = NULL;

        // Call shell again
        const uint8_t cmd[FILENAME_SIZE] = "shell";
        execute(cmd);
    
        return 0;
    }

    /* Restore parent paging */
    page_dir[USER_INDEX].page_table_address = (KERNEL_END + (curr_pcb->parent_pcb->pid * FOURMB_BITS)) >> ADDRESS_SHIFT;
    flush_tlb();

    /* Clear file descriptors */
    int i;
    for (i = 0; i < MAX_OPEN_FILES; i++) {
        if (curr_pcb->fds[i].flags == FD_USED) {
            close(i);
        }
    }

    /* Write parent's process info back to TSS */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_END - (curr_pcb->parent_pcb->pid * EIGHTKB_BITS);
    
    /* Jump to execute return */
    uint32_t saved_ebp = curr_pcb->saved_ebp;

    /* Unset current PID in pids and restore curr_pcb */
    pids[curr_pcb->pid] = 0;
    curr_pcb = curr_pcb->parent_pcb;

    /* Switch back to `execute`'s stack by setting ebp to saved_ebp */
    // Return from execute function
    asm volatile("          \n\
        movl %0, %%ebp      \n\
                            \n\
        movl %1, %%eax      \n\
        leave               \n\
        ret                 \n\
        "
        :
        : "r"(saved_ebp), "r"(status_32)
    );

    return -1;
}

/* int32_t execute(const uint8_t* command)
 *   DESCRIPTION: Executes a new process by setting up iret, paging, and other paramaters. Checks to 
 *                 make sure that the process can start and loads program into memory.
 * 
 *   INPUTS: const uint8_t* command       : the command passed in terminal
 *   OUTPUTS: none
 *   RETURN VALUE: status of the execute
 *   SIDE EFFECTS: Starts a process if its able to
 */
int32_t execute(const uint8_t* command) {
    int i;

    /* Parse args */
    uint8_t file_name[BUFFER_SIZE];
    uint8_t args[BUFFER_SIZE];
    memset(file_name, 0, sizeof(file_name));
    memset(args, 0, sizeof(args));

    // Skip any spaces at the start
    int command_idx = 0;
    while (command[command_idx] == ' ') { command_idx++; }

    // Get the command
    for (i = 0; i < BUFFER_SIZE - 1; i++, command_idx++) {
        if (command[command_idx] == ' ' || command[command_idx] == '\0') {
            break;
        }
        file_name[i] = command[command_idx];
    }

    while (command[command_idx] == ' ') { command_idx++; }

    for (i = 0; i < BUFFER_SIZE - 1; i++, command_idx++) {
        if (command[command_idx] == '\0') {
            break;
        }
        args[i] = command[command_idx];
    }

    /* Check File Validity */
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

    /* Find PID */
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

    //update parent and current pcb
    pcb_t* parent_pcb = curr_pcb;
    curr_pcb = (pcb_t*) (KERNEL_END - (EIGHTKB_BITS * (pid + 1)));
    curr_pcb->pid = pid;
    curr_pcb->parent_pcb = parent_pcb;
    memcpy(curr_pcb->args, args, sizeof(args));
    curr_pcb->exception_occured = 0;

    /* Setup Paging */

    // make virtual mem map to right physical address
    page_dir[USER_INDEX].page_table_address = (KERNEL_END + (curr_pcb->pid * FOURMB_BITS)) >> ADDRESS_SHIFT;
    flush_tlb();
        
    /* Load file into memory */

    // know it's an executable now copy to physical address
    uint8_t* program_location = (uint8_t*) (USER_ADDRESS + USER_OFFSET);
    uint32_t file_size = ((inodes_t *)((uint8_t *)file_system + ((dentry.inode + 1) * (BLOCK_SIZE))))->length;
    read_data(dentry.inode, 0, program_location, file_size);

    uint32_t eip = *((uint32_t*) (program_location + EIP_OFFSET));

    /* Create pcb and clear fds */

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

    /* Prepare For Context Switch */
    // save current ebp
    register uint32_t saved_ebp asm("ebp");
    curr_pcb->saved_ebp = saved_ebp;

    // modify esp0 and ss0 in TSS
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_END - (curr_pcb->pid * EIGHTKB_BITS);

    // push IRET context on the the correct order and call iret
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


/* int32_t read(int32_t fd, void* buf, int32_t nbytes)
* Inputs: int32_t fd - file descriptor
*         void* buf - buffer
*          int32_t nbytes - number of bytes to read
* Return Value: int32_t -> number of bytes read
* Function: reads data in from file and buffer
*/
int32_t read(int32_t fd, void* buf, int32_t nbytes) {
    // check for invalid arguments
    if(fd < 0 || fd >= MAX_OPEN_FILES
       || buf == NULL || nbytes < 0
       || curr_pcb->fds[fd].flags == FD_AVAIL) {
        return -1;
    }

    // read if possible
    if (curr_pcb->fds[fd].functions.read == NULL) {
        return 0;
    }
    //return the number of bytes read
    return curr_pcb->fds[fd].functions.read(fd, buf, nbytes);
}

/* int32_t write(int32_t fd, void* buf, int32_t nbytes)
 *   Inputs:    int32_t fd - file descriptor
 *              void* buf - buffer
*               int32_t nbytes - number of bytes to write
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if not valid or the numbe number of bytes to writeytes written
 *   SIDE EFFECTS: Writes to a buffer given the numb
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
    // check for invalid arguments
    if (fd < 0 || fd >= MAX_OPEN_FILES
       || buf == NULL || nbytes < 0
       || curr_pcb->fds[fd].flags == FD_AVAIL) {
        return -1;
    }

    // check if the function is null}
    if (curr_pcb->fds[fd].functions.write == NULL) {
        return 0;
    }
    // return the number of bytes read
    return curr_pcb->fds[fd].functions.write(fd,buf,nbytes);
}

/* int32_t open(const uint8_t* filename)
* Inputs: const uint8_t* filename - name of file to open
* Return Value: int32_t -> file descriptor index
* Function: opens a file
*/
int32_t open(const uint8_t* filename) {

    //check if the file is valid
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

    //copy file data
    curr_pcb->fds[fd_idx].inode = dentry.inode;
    curr_pcb->fds[fd_idx].pos = 0;
    curr_pcb->fds[fd_idx].flags = FD_USED;

    // switch to the right function call given the type
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
    default:
        return -1;
    }

    // if valid then open the file
    if (curr_pcb->fds[fd_idx].functions.open != NULL) {
        curr_pcb->fds[fd_idx].functions.open(filename);
    }

    return fd_idx;
} 

/* int32_t close(int32_t fd)
* Inputs: int32_t fd - name of file to open
* Return Value: int32_t -> success/fail
* Function: closes a file and sets it available
*/
int32_t close(int32_t fd) {
    // check for valid arguments
    if(fd < 0 || fd >= MAX_OPEN_FILES || curr_pcb->fds[fd].flags == FD_AVAIL) {
        return -1;
    }

    if (curr_pcb->fds[fd].functions.close == NULL) {
        return 0;
    }

    //close the file
    uint32_t ret = curr_pcb->fds[fd].functions.close(fd);

    curr_pcb->fds[fd].functions.open = NULL;
    curr_pcb->fds[fd].functions.close = NULL;
    curr_pcb->fds[fd].functions.read = NULL;
    curr_pcb->fds[fd].functions.write = NULL;
    curr_pcb->fds[fd].inode = 0;
    curr_pcb->fds[fd].pos = 0;
    curr_pcb->fds[fd].flags = FD_AVAIL;

    return ret;
}

/* int32_t getargs(uint8_t* buf, int32_t nbytes)
* Inputs: uint8_t* buf - buffer to write to
*         int32_t nbytes - number of bytes to write
* Return Value: int32_t -> success/fail
* Function: copies shell arguments into given buffer
*/
int32_t getargs(uint8_t* buf, int32_t nbytes) {
    if (buf == NULL || curr_pcb->args[0] == '\0') {
        return -1;
    }

    strncpy((int8_t*) buf, (const int8_t*) curr_pcb->args, nbytes);
    return 0;
}

int32_t vidmap(uint8_t** screen_start) {
    printf("vidmap called - not implemented\n");
    return -1;
}

int32_t set_handler(int32_t signum, void* handler_address) {
    printf("set_handler called - not implemented\n");
    return -1;
}

int32_t sigreturn(void) {
    printf("sigreturn calle - not implemented\n");
    return -1;
}
