#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#include "types.h"
#include "x86_desc.h"

#define BLOCK_SIZE 4096
#define DATA_BLOCK_NUM 1023
#define FILENAME_SIZE 32
#define DIR_ENTRY_RESERVED 24
#define BOOT_BLOCK_RESERVED 52
#define DIR_ENTRIES_NUM 63
#define ELF_SIZE 4

/* File descriptor flags */
#define FD_USED 1  // FD in use
#define FD_AVAIL 0 // FD available

/* directory_entries / dentry struct */
typedef struct dentry {
    uint8_t file_name[FILENAME_SIZE];
    uint32_t file_type;
    uint32_t inode;
    uint8_t reserved[DIR_ENTRY_RESERVED];
} dentry_t;

/* data block struct */
typedef struct boot_block {
    uint32_t num_dir_entries;
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    uint8_t reserved[BOOT_BLOCK_RESERVED];
    dentry_t dir_entries[DIR_ENTRIES_NUM];
} boot_block_t;

/* inode struct */
typedef struct inodes {
    uint32_t length;
    uint32_t data_block[DATA_BLOCK_NUM];
} inodes_t;

/* data block struct */
typedef struct data_block {
    uint8_t data[BLOCK_SIZE];
} data_block_t;

/* function pointer struct */
typedef struct func_pt {
    int32_t (*open)(const uint8_t *filename);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void *buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void *buf, int32_t nbytes);
} func_pt_t;

/* file descriptior */
typedef struct fd {
    func_pt_t functions;
    uint32_t inode;
    uint32_t pos; // needs to be updated in each "read" call
    uint32_t flags;
} fd_t;

/* Intialize file system*/
extern boot_block_t *file_system;

/* dentry functions */
extern uint32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry);
extern uint32_t read_dentry_by_index(uint32_t index, dentry_t *dentry);
extern uint32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length);

/* File functions */
extern int32_t file_read(int32_t fd, void *buf, int32_t nbytes);
extern int32_t file_write(int32_t fd, const void *buf, int32_t nbytes);
extern int32_t file_open(const uint8_t *filename);
extern int32_t file_close(int32_t fd);

/* Directory functions */
extern int32_t dir_read(int32_t fd, void *buf, int32_t nbytes);
extern int32_t dir_write(int32_t fd, const void *buf, int32_t nbytes);
extern int32_t dir_open(const uint8_t *filename);
extern int32_t dir_close(int32_t fd);

/* Function structs */
extern func_pt_t make_file_fops(void);
extern func_pt_t make_dir_fops(void);

#endif // _FILE_SYSTEM_H
