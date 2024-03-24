#include "file_system.h"
#include "lib.h"


// init file functions in standard interface
functions_t file = {
    .open = f_open,
    .close = f_close,
    .read = f_read,
    .write = f_write
};

// init directory functions in standard interface
functions_t directory = {
    .open = d_open,
    .close = d_close,
    .read = d_read,
    .write = d_write
};

// init file systems by loading the address of module 0
void file_system_init(uint32_t * address) {
    file_system = (boot_block_t *) address;
}

uint32_t read_dentry_by_name (const uint8_t* fname, dentry_t * dentry) {
    // find the file (if it exsits by name)
    int i, j, same;

    // check each directory
    for(i = 0; i < file_system->num_dir_entries; i++) {
        
        // compare file name
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

uint32_t read_dentry_by_index (uint32_t index, dentry_t * dentry) {
    // check if index is valid
    if(index >= file_system->num_dir_entries || index < 0) { return -1;}
    //if the same than update dentry argument with file name, type, and inode
    memcpy(dentry->file_name, file_system->dir_entires[index].file_name, FILENAME_SIZE);
    dentry->file_type = file_system->dir_entires[index].file_type;
    dentry->inode = file_system->dir_entires[index].inode;

    return 0;
}

uint32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    int inode_block_index, data_index, read_count, block_index;

    // check for garbage values
    if(buf == NULL) { return -1; }

    // calculate the starting point
    inode_block_index = (uint32_t)(offset / BLOCK_SIZE);
    data_index = offset % BLOCK_SIZE;

    // check if inode and start_data are valid
    if(inode >= file_system->num_inodes || inode < 0) {return -1; }

    // read data for the given length

    // get easy access to correct starting inode_t and block_data struct
    inodes_t * inode_array = (inodes_t*)(((uint32_t*)file_system + BLOCK_SIZE) + (inode * sizeof(inodes_t)));
    uint32_t len = inode_array->length;
    
    // check if index for block index is valid
    if(inode_block_index >= ((len / BLOCK_SIZE) + 1)) {return -1;}

    block_index = inode_array->data_block[inode_block_index]; 

    // get access to first data block
    data_block_t * data_bloc_array = (data_block_t *) ((uint32_t *)inode_array + (file_system->num_inodes * BLOCK_SIZE));


    // copy data until length reached or invalid location
    while (length > 0) 
    {
        length--;
        
        // copy data
        buf[read_count] = ((data_block_t *)(((uint32_t *)data_bloc_array + block_index*(BLOCK_SIZE))))->data[data_index];
        read_count++;

        // figure out if we need to jump to next data block (and do so if its valid)
        data_index++;

        if(data_index % BLOCK_SIZE == 0) { 
            // look at next data block
            inode_block_index++; 

            if(inode_block_index >= ((len / BLOCK_SIZE) + 1)) {return -1;}

            block_index = inode_array->data_block[inode_block_index]; 

            data_index = 0;
        }
    }

    // return the total bits written;
    return read_count;
}

// reads file contents into the buffer
uint32_t f_read (uint32_t fd, void * buf, uint32_t nbytes) {
    // since fd is for next checkpoint use as inode
    return read_data(fd, 0, buf, nbytes);
}

// reads oen file from the dir into the buffer
uint32_t d_read (uint32_t fd, void * buf, uint32_t nbytes) {
    // since fd is not given use fd as index amd nbytes is ingnored
    dentry_t file = file_system->dir_entires[fd];

    // find the details about the file and write into buffer

    // JUST NAME FOR NOW!!!
    uint8_t * buffer = (uint8_t * ) buf;

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

void d_read_test() {
    int i, j;
	uint32_t count = file_system->num_dir_entries;

	for(i = 0; i < count; i++) {
        uint8_t temp[FILENAME_SIZE];
		d_read(i, temp, FILENAME_SIZE);
        printf("File name: ");
        for(j = 0; j < FILENAME_SIZE; j++) {
            printf("%c", temp[j]);
        }

        printf("\n");
		
	}
}

void f_read_test() {
    int i;
    uint8_t buf[200] = {0};

    f_read(6, buf, 200);

    for(i = 0; i < 200; i++) {
        printf("%c", buf[i]);
    }
    printf("DONE!");
}