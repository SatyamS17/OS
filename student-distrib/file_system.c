#include "file_system.h"
#include "lib.h"


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

    while(length > 0) {
        length--;
        
        buf[read_count] = (data_block[block_index]).data[data_index];
        read_count++;
        data_index++;

        if(data_index % BLOCK_SIZE == 0) { 
            // look at next data block
            inode_block_index++; 

            if( inode_block_index >= ((file_inode->length / BLOCK_SIZE) + 1)) {return file_inode->length;}
            
            // make sure next data block is valid
            block_index = file_inode->data_block[inode_block_index];
            
            data_index = 0;
        }
        
    }
    // return the total bits written;
    return file_inode->length;
}

// reads file contents into the buffer
uint32_t f_read (uint32_t fd, void * buf, uint32_t nbytes) {
    // check for garbage values
    if(buf == NULL) { return -1; }

    // since fd is for next checkpoint use as inode
    return read_data(fd, 0, buf, nbytes);
}

// reads oen file from the dir into the buffer
uint32_t d_read (uint32_t fd, void * buf, uint32_t nbytes) {
    static uint32_t index = 0;
    dentry_t file = file_system->dir_entires[index];

    index = (index + 1) % file_system->num_dir_entries;
    // find the details about the file and write into buffer

    // JUST NAME FOR NOW!!!
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

void d_read_test() {
    int i, j;

    printf("TESTING DENTRY READ: \n");
	for(i = 0; i < file_system->num_dir_entries; i++) {
        uint8_t temp[FILENAME_SIZE];
		d_read(0, temp, FILENAME_SIZE);
        printf("File name: ");
        for(j = 0; j < FILENAME_SIZE; j++) {
            printf("%c", temp[j]);
        }

        printf("\n");
		
	}
}

void f_read_long_test() {
    uint8_t test_name[FILENAME_SIZE] = "verylargetextwithverylongname.tx";
    f_read_test(test_name);
}

void f_read_short_test() {
    uint8_t test_name[FILENAME_SIZE] = "frame0.txt";
    f_read_test(test_name);
}

void f_read_exec_test() {
    uint8_t test_name[FILENAME_SIZE] = "ls";
    f_read_test(test_name);
}


void f_read_test(uint8_t * test_name) {
    int i;
    dentry_t test_dentry;

    read_dentry_by_name(test_name, &test_dentry);

    uint32_t size = ((inodes_t *)((uint8_t *)file_system + ((test_dentry.inode + 1) * (BLOCK_SIZE))))->length;

    uint8_t buf[BLOCK_SIZE*20] = {0};

    uint32_t data = read_data(test_dentry.inode, 0, buf, size);
    //printf("%d", data);
    for(i = 0; i < data; i++) {
        if(buf[i] != '\0') {
           putc(buf[i]); 
        }
    }
    
    printf("\nFile read: ");
    for(i = 0; i < FILENAME_SIZE; i++) { putc(test_dentry.file_name[i]);}
    
}


void test_read_dentry_index() {
    dentry_t test_dentry;
    int i;
    printf("TESTING READ DENTRY BY INDEX: \n");

    for(i = 0; i < file_system->num_dir_entries; i++) {
        // test read_dentry
        read_dentry_by_index(i, &test_dentry);

        printf("File %d name: %s   ", i, test_dentry.file_name);
        printf("type: %d   ", test_dentry.file_type);
        printf("inode: %d \n", test_dentry.inode);
    }

}


void test_read_dentry_name() {
    dentry_t test_dentry;
    int i;

    uint8_t test_names[5][FILENAME_SIZE] = {"ls", "cat", "notreal", "", "frame0.txt"};

    printf("TESTING READ DENTRY BY NAME: \n");
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

