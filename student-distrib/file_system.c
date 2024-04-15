#include "file_system.h"

#include "lib.h"
#include "syscall.h"

uint8_t elf[ELF_SIZE] = {0x7f, 0x45, 0x4c, 0x46};

boot_block_t* file_system = NULL;

/* file_system_init
 *   DESCRIPTION: Loads the address of module 0 into file system pointer
 *    
 *   INPUTS: uint32_t * address : address of module 0
 *   OUTPUTS: none
 *   RETURN VALUE: void
 *   SIDE EFFECTS: Initalizes the starting address of file system
 */
void file_system_init(uint32_t * address) {
    // init file systems by loading the address of module 0
    file_system = (boot_block_t *) address;
}

/* read_dentry_by_name
 *   DESCRIPTION: Tries to search for the right dentry based on the file name.
 *                Copies file name, type, and inode into dentry struct if found
 * 
 *   INPUTS: const uint8_t* fname : pointer to file name string
 *           dentry_t * dentry    : pointer to dentry
 *   OUTPUTS: none
 *   RETURN VALUE: 0 : if worked -1 : invalid arguments or not found
 *   SIDE EFFECTS: Updates dentry struct with new 
 */
uint32_t read_dentry_by_name (const uint8_t* fname, dentry_t * dentry) {
    // check for garbage values
    if (dentry == NULL || fname == NULL) { return -1; }

    // find the file (if it exsits by name)
    int i;

    // check each directory
    for(i = 0; i < file_system->num_dir_entries; i++) {
        uint32_t len1 = strlen((const int8_t*) file_system->dir_entries[i].file_name);
        uint32_t len2 = strlen((const int8_t*) fname);

        if (len2 != FILENAME_SIZE && len1 != len2) {
            continue;
        }
        if (strncmp((const int8_t*) fname, (const int8_t*) file_system->dir_entries[i].file_name, len2) != 0) {
            continue;
        }

        // if the same than update dentry argument with file name, type, and inode
        memcpy(dentry->file_name, file_system->dir_entries[i].file_name, FILENAME_SIZE);
        dentry->file_type = file_system->dir_entries[i].file_type;
        dentry->inode = file_system->dir_entries[i].inode;

        return 0;
    }

    return -1;
}

/* read_dentry_by_index
 *   DESCRIPTION: Tries to search for the right dentry based on index.
 *                Copies file name, type, and inode into dentry struct if found
 * 
 *   INPUTS: uint32_t index       : index to be searched
 *           dentry_t * dentry    : pointer to dentry
 *   OUTPUTS: none
 *   RETURN VALUE: 0 : if worked -1 : invalid arguments or not found
 *   SIDE EFFECTS: Updates dentry struct with new 
 */
uint32_t read_dentry_by_index (uint32_t index, dentry_t * dentry) {
    // check for garbage values
    if(dentry == NULL) { return -1; }

    // check if index is valid
    if(index >= file_system->num_dir_entries || index < 0) { return -1;}

    //if the same than update dentry argument with file name, type, and inode
    memcpy(dentry->file_name, file_system->dir_entries[index].file_name, FILENAME_SIZE);
    dentry->file_type = file_system->dir_entries[index].file_type;
    dentry->inode = file_system->dir_entries[index].inode;

    return 0;
}

/* read_data
 *   DESCRIPTION: Copies data from a file into a buffer based on the length or
 *              until EOF is reached.
 * 
 *   INPUTS: uint32_t inode       : inode of file 
 *           uint32_t offset      : starting offset for copying data
 *           uint8_t * buf        : pointer to buffer to copy data in
 *           uint32_t length      : number of bytes to copy
 *   OUTPUTS: none
 *   RETURN VALUE: The number of bytes read
 *   SIDE EFFECTS: Updates buffer with data
 */
uint32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    uint32_t inode_block_index, data_index, block_index;
    uint32_t read_count = 0;

    // check if inode index is valid
    if (inode >= file_system->num_inodes || inode < 2) { return -1; }

    //calulcate starting point
    inode_block_index = (uint32_t)(offset / BLOCK_SIZE);
    data_index = offset % BLOCK_SIZE;

    //find correct inode based on index
    inodes_t * file_inode = (inodes_t *)((uint8_t *)file_system + ((inode + 1) * (BLOCK_SIZE)));
    // check if starting index fits in length
    if( inode_block_index >= ((file_inode->length / BLOCK_SIZE) + 1)) {return -1;}

    // find start of data blocks
    data_block_t * data_block = (data_block_t *)((uint8_t *)file_system + ((file_system->num_inodes + 1) * BLOCK_SIZE));
    block_index = file_inode->data_block[inode_block_index];

    // keep copying data until EOF or desired length is reached
    int i;
    for (i = 0; i < length; i++) {
        if (read_count + offset >= file_inode->length) {
            break;
        }
        
        //copy data over to the buffer
        buf[read_count] = (data_block[block_index]).data[data_index];
        read_count++;
        data_index++;

        // Check if reached the end of data for the block
        if(data_index % BLOCK_SIZE == 0) { 
            // look at next data block index
            inode_block_index++; 

            //check if it's valid
            if( inode_block_index >= ((file_inode->length / BLOCK_SIZE) + 1)) {return file_inode->length;}
            
            // point to first point in next data block
            block_index = file_inode->data_block[inode_block_index];
            data_index = 0;
        }
        
    }
    // return the total bits written;
    return read_count;
}

/* f_read
 *   DESCRIPTION: Copies data from a file into a buffer based on the length or
 *              until EOF is reached. Calls read_data.
 * 
 *   INPUTS: uint32_t fd          : index of file descriptor
 *           uint8_t * buf        : pointer to buffer to copy data in
 *           uint32_t nbytes      : number of bytes to copy
 *   OUTPUTS: none
 *   RETURN VALUE: The number of bytes read
 *   SIDE EFFECTS: Updates buffer with data
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {
    // check for garbage values
    if (buf == NULL) { return -1; }
    if (fd < 2 || fd >= MAX_OPEN_FILES) { return -1; }

    // will this cause a problem if len is too long?
    uint32_t ret = read_data(curr_pcb->fds[fd].inode, curr_pcb->fds[fd].pos, buf, nbytes);
    curr_pcb->fds[fd].pos += ret;
    return ret;
}

/* d_read
 *   DESCRIPTION: Copies the next file name in the directory into buffer.
 * 
 *   INPUTS: uint32_t fd          : index of file descriptor
 *           uint8_t * buf        : pointer to buffer to copy data in
 *           uint32_t nbytes      : number of bytes to copy
 *   OUTPUTS: none
 *   RETURN VALUE: The number of bytes read
 *   SIDE EFFECTS: Updates buffer with data
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes) {
    // check for garbage values
    if (buf == NULL) { return -1; }
    if (fd < 2 || fd >= MAX_OPEN_FILES) { return -1; }
    if (curr_pcb == NULL) { return -1; }

    // keep track of the index being printed since once at a time
    if (curr_pcb->fds[fd].pos >= file_system->num_dir_entries) { return 0; }
    dentry_t file = file_system->dir_entries[curr_pcb->fds[fd].pos];

    // find the details about the file and write into buffer
    uint8_t * buffer = (uint8_t *) buf;

    // look at next file
    int i;
    for(i = 0; file.file_name[i] != '\0' && i < FILENAME_SIZE; i++) {
        buffer[i] = file.file_name[i];
    }

    // increase pos to look at next file
    curr_pcb->fds[fd].pos++;

    return i;
}

/* does nothing for this checkpoint*/
int32_t file_open(const uint8_t* filename) { return 0; }
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) { return 0; }
int32_t file_close(int32_t fd) { return 0; }

int32_t dir_open(const uint8_t* filename) { return 0; }
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes) { return 0; }
int32_t dir_close(int32_t fd) { return 0; }

/* make_file_fops
 *   DESCRIPTION: Initalizes file functions
 * 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: func_pt_t struct of pointers to functions
 *   SIDE EFFECTS: Creates file functions
 */
func_pt_t make_file_fops(void) {
    func_pt_t f;
    f.open = file_open;
    f.close = file_close;
    f.read = file_read;
    f.write = file_write;
    return f;
}

/* make_dir_fops
 *   DESCRIPTION: Initalizes directory functions
 * 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: func_pt_t struct of pointers to functions
 *   SIDE EFFECTS: Creates file functions
 */
func_pt_t make_dir_fops(void) {
    func_pt_t f;
    f.open = dir_open;
    f.close = dir_close;
    f.read = dir_read;
    f.write = dir_write;
    return f;
}
