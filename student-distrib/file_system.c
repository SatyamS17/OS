#include "file_system.h"
#include "lib.h"

#define RUN_TESTS
uint8_t elf[ELF_SIZE] = {0x7f, 0x45, 0x4c, 0x46};

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

    // init file function pointers
    file_func.open = f_open;
    file_func.close = f_close;
    file_func.read = f_read;
    file_func.write = f_write;
    // init directory function pointers
    dir_func.open = d_open;
    dir_func.close = d_close;
    dir_func.read = d_read;
    dir_func.write = d_write;
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
    if(dentry == NULL || fname == NULL) { return -1; }

    // find the file (if it exsits by name)
    int i, j, same;

    // check each directory
    for(i = 0; i < file_system->num_dir_entries; i++) {
        
        // compare file names and keep track of flag
        same = 1;
        for(j = 0; j < FILENAME_SIZE; j++) {
            if(fname[j] != file_system->dir_entires[i].file_name[j]) {
                same = 0;
                break;
            }
        }

        // move on if not the same
        if(!same) { continue; }

        // if the same than update dentry argument with file name, type, and inode
        memcpy(dentry->file_name, file_system->dir_entires[i].file_name, FILENAME_SIZE);
        dentry->file_type = file_system->dir_entires[i].file_type;
        dentry->inode = file_system->dir_entires[i].inode;

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
    int i;

    // check if index is valid
    if(index >= file_system->num_dir_entries || index < 0) { return -1;}

    //if the same than update dentry argument with file name, type, and inode
    for(i = 0; i < FILENAME_SIZE; i++) {
        dentry->file_name[i] = file_system->dir_entires[index].file_name[i];
    }    
    dentry->file_type = file_system->dir_entires[index].file_type;
    dentry->inode = file_system->dir_entires[index].inode;

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
uint32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    uint32_t inode_block_index, data_index, block_index;
    uint32_t read_count = 0;

    // check if inode index is valid
    if(inode >= file_system->num_inodes || inode < 0) {return -1;}

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
    while(length > 0) {
        length--;
        
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
uint32_t f_read (uint32_t fd, void * buf, uint32_t nbytes) {
    // check for garbage values
    if(buf == NULL) { return -1; }

    // since fd is for next checkpoint use as inode
    return read_data(fd, 0, buf, nbytes);
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
uint32_t d_read (uint32_t fd, void * buf, uint32_t nbytes) {
    // check for garbage values
    if(buf == NULL) { return -1; }

    // kepp track of the index being printed since once at a time
    static uint32_t index = 0;
    dentry_t file = file_system->dir_entires[index];
    index = (index + 1) % file_system->num_dir_entries;

    // find the details about the file and write into buffer
    uint8_t * buffer = (uint8_t *) buf;
    memcpy(buffer, file.file_name, FILENAME_SIZE);

    return 0;
}

/* load_program
 *   DESCRIPTION: load program by copying excutable into physical memory
 * 
 *   INPUTS: uint32_t * location        : address of which to copy to
 *           uint8_t * file_name        : pointer to file name array
 *           uint32_t * eip             : pointer to eip (copied bytes 24-27)
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if worked or -1 if it didn't
 *   SIDE EFFECTS: Loads program into memeory
 */
uint8_t load_program(uint8_t * file_name, uint8_t * location, uint32_t * eip) {
    // check for garbage values
    if(file_name == NULL || location == NULL || eip == NULL) {return -1;}

    // get acess to file if it exists
    dentry_t file;

    if(read_dentry_by_name(file_name, &file) == -1) {return -1;}

    // check if the file is an executable
    uint8_t * exec_check;
    int i;

    if(read_data(file.inode, 0, exec_check, ELF_SIZE) != ELF_SIZE) { return -1; }
    // check for the magic numbers
    for(i = 0; i < ELF_SIZE; i++) {
        if(exec_check[i] != elf[i]) { return -1; }
    }

    // know it's an executable now copy to physical address
    uint32_t file_size = ((inodes_t *)((uint8_t *)file_system + ((file.inode + 1) * (BLOCK_SIZE))))->length;
    read_data(file.inode, 0, location, file_size);

    // bytes 24-27 should be updated in eip (so offset location by 24 bytes)
    eip = (uint32_t *)(location + 24);

    return 0;
}


/* does nothing for this checkpoint*/
uint32_t f_open (const uint8_t * filename) { return 0; }
uint32_t f_write (uint32_t fd, const void* buf, uint32_t nbytes) { return 0; }
uint32_t f_close (uint32_t fd) { return 0; }

uint32_t d_open (const uint8_t * filename) { return 0; }
uint32_t d_write (uint32_t fd, const void* buf, uint32_t nbytes) { return 0; }
uint32_t d_close (uint32_t fd) { return 0; }
