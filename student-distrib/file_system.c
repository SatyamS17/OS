#include "file_system.h"
#include "lib.h"

#define RUN_TESTS


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
    return file_inode->length;
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

/* does nothing for this checkpoint*/
uint32_t f_open (const uint8_t * filename) { return 0; }
uint32_t f_write (uint32_t fd, const void* buf, uint32_t nbytes) { return 0; }
uint32_t f_close (uint32_t fd) { return 0; }

uint32_t d_open (const uint8_t * filename) { return 0; }
uint32_t d_write (uint32_t fd, const void* buf, uint32_t nbytes) { return 0; }
uint32_t d_close (uint32_t fd) { return 0; }

/* TEST FUNCTIONS*/

#ifdef RUN_TESTS

/* Directory Read
 * 
 * Prints out all the files in the directory
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
void d_read_test() {
    int i, j;

    // Prints out each of the files in the directory
	for(i = 0; i < file_system->num_dir_entries; i++) {
        uint8_t temp[FILENAME_SIZE];
        // calls function being tested
		d_read(0, temp, FILENAME_SIZE);
        printf("File name: ");

        for(j = 0; j < FILENAME_SIZE; j++) {
            printf("%c", temp[j]);
        }

        printf("\n");
	}
}

/* File Long Read
 * 
 * Prints out contents of the file
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
void f_read_long_test() {
    uint8_t test_name[FILENAME_SIZE] = "verylargetextwithverylongname.tx";
    f_read_test(test_name);
}

/* File Short Read
 * 
 * Prints out contents of the file
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
void f_read_short_test() {
    uint8_t test_name[FILENAME_SIZE] = "frame0.txt";
    f_read_test(test_name);
}

/* File Executable Read
 * 
 * Prints out contents of the file
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
void f_read_exec_test() {
    uint8_t test_name[FILENAME_SIZE] = "ls";
    f_read_test(test_name);
}

/* File Not-Readable Read
 * 
 * Prints out contents of the file
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
void f_read_noread_test() {
    uint8_t test_name[FILENAME_SIZE] = ".";
    f_read_test(test_name);
}

/* File Read
 * 
 * Prints out contents of the file
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
void f_read_test(uint8_t * test_name) {
    int i;
    dentry_t test_dentry;

    // get file information based on inode so that we can read correct file
    read_dentry_by_name(test_name, &test_dentry);

    // get the size of the file
    uint32_t size = ((inodes_t *)((uint8_t *)file_system + ((test_dentry.inode + 1) * (BLOCK_SIZE))))->length;

    // create a large buffer that can hold large files (*20 makes sure the buffer is large for testing)
    uint8_t buf[BLOCK_SIZE*20] = {0};

    // if its a readable file then read it
    if(test_dentry.file_type == 2) {
        // fill buffer with data
        uint32_t data = f_read(test_dentry.inode, buf, size);

        //print buffer
        for(i = 0; i < data; i++) {
            if(buf[i] != '\0') {
            putc(buf[i]); 
            }
        }
    
        printf("\nFile read: ");
    }
    // else print out error that it's not readable
    else {
        printf("\nNot readable file : ");
    }

    // print the name of the file
    for(i = 0; i < FILENAME_SIZE; i++) { putc(test_dentry.file_name[i]);}
    putc('\n');
}

/* Read Dentry by Index
 * 
 * Prints out inode based on index if its valid else error
 * Inputs: None
 * Outputs:None
 */
void test_read_dentry_index() {
    dentry_t test_dentry;
    int i;

    // check both valid and invalid indexs for dentry
    for(i = 0; i < file_system->num_dir_entries + 3; i++) {
        // test read_dentry_by_index
        if(read_dentry_by_index(i, &test_dentry)== 0 ) {
            printf("File %d name: %s   ", i, test_dentry.file_name);
            printf("type: %d   ", test_dentry.file_type);
            printf("inode: %d \n", test_dentry.inode);
        }else {
            printf("Index %d not valid \n", i);
        }
    }

}

/* Read Dentry by Name
 * 
 * Prints out inode based on file name if its valid else error
 * Inputs: None
 * Outputs:None
 */
void test_read_dentry_name() {
    dentry_t test_dentry;
    int i;

    // can change but good selection of test cases
    uint8_t test_names[5][FILENAME_SIZE] = {"ls", "cat", "notreal", "", "frame0.txt"};

    // check both valid and invalid file names for dentry
    for(i = 0; i < 5; i++) {
        // test read_dentry
        if(read_dentry_by_name(test_names[i], &test_dentry) == 0) {
            printf("File %d name: %s  ", i, test_dentry.file_name);
            printf("type: %d   ", test_dentry.file_type);
            printf("inode: %d \n", test_dentry.inode);
        } else {
            printf("Invalid file!!! Name: %s  \n", test_names[i]);
        }
    }

}

#endif
