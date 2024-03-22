#include "file_system.h"
#include "lib.h"

// start at module 0
void file_system_init(uint32_t address) {
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
            if(fname[j] != file_system->dir_entires[i]->file_name[j]) {
                same = 0;
                break;
            }
        }

        // move on if not the same
        if(!same) {
            continue;
        }

        // if the same than update dentry argument with file name, type, and inode
        memcpy(dentry->file_name, file_system->dir_entires[i]->file_name, FILENAME_SIZE);
        dentry->file_type = file_system->dir_entires[i]->file_type;
        dentry->inode = file_system->dir_entires[i]->inode;

        return 0;
    }

    return -1;
}

uint32_t read_dentry_by_index (uint32_t index, dentry_t * dentry) {
    // check if index is valid
    if(index >= file_system->num_dir_entries || index < 0) {
        return -1;
    }
    // if the same than update dentry argument with file name, type, and inode
    memcpy(dentry->file_name, file_system->dir_entires[index]->file_name, FILENAME_SIZE);
    dentry->file_type = file_system->dir_entires[index]->file_type;
    dentry->inode = file_system->dir_entires[index]->inode;

    return 0;
}

uint32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {

    // check if inode is valid
    if(inode >= file_system->num_inodes || inode < 0) {
        return -1;
    }
    
    // FINSIH THIS !!!!!!!!!!!!!!!!!!!!!!!!!! ------------------------!!!!!!!!!!!!!!


    return 0;
}
