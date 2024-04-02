#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#ifndef ASM

#include "types.h"
#include "x86_desc.h"

#define BLOCK_SIZE              4096
#define DATA_BLOCK_NUM          1023
#define FILENAME_SIZE           32
#define DIR_ENTRY_RESERVED      24
#define BOOT_BLOCK_RESERVED     52
#define DIR_ENTRIES_NUM         63
#define ELF_SIZE                4

// File descriptor flags
#define FD_USED  1  // FD in use
#define FD_AVAIL 0  // FD available
// FILE SYSTEM ARRAY ----------------------------------------


/* directory_entries / dentry struct */
typedef struct dentry_t
{
    uint8_t  file_name[FILENAME_SIZE];
    uint32_t file_type;
    uint32_t inode;
    uint8_t  reserved[DIR_ENTRY_RESERVED];
} dentry_t;

/* data block struct */
typedef struct boot_block_t
{
    uint32_t num_dir_entries;
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    uint8_t  reserved[BOOT_BLOCK_RESERVED];
    dentry_t dir_entires[DIR_ENTRIES_NUM];
} boot_block_t;

/* inode struct */
typedef struct inodes_t
{
    uint32_t length;
    uint32_t data_block[DATA_BLOCK_NUM];
} inodes_t;

/* data block struct */
typedef struct data_block_t
{
    uint8_t data[BLOCK_SIZE];
} data_block_t;

// FILE SYSTEM ARRAY ----------------------------------------

/* file descriptior */
typedef struct fd_t 
{
    func_pt_t functions;
    uint32_t inode;
    uint32_t pos;   // needs to be updated in each "read" call
    uint32_t flags;
} fd_t;

/* Intialize file system*/
boot_block_t * file_system;
void file_system_init(uint32_t * address);

/* dentry functions */
uint32_t read_dentry_by_name (const uint8_t* fname, dentry_t * dentry);
uint32_t read_dentry_by_index (uint32_t index, dentry_t * dentry);
uint32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* File functions */
uint32_t f_read (uint32_t fd, void * buf, uint32_t nbytes);
uint32_t f_write (uint32_t fd, const void* buf, uint32_t nbytes);
uint32_t f_open (const uint8_t * filename);
uint32_t f_close (uint32_t fd);

/* Directory functions */
uint32_t d_read (uint32_t fd, void * buf, uint32_t nbytes);
uint32_t d_write (uint32_t fd, const void* buf, uint32_t nbytes);
uint32_t d_open (const uint8_t * filename);
uint32_t d_close (uint32_t fd);

/* function structs */
func_pt_t file_func;
func_pt_t dir_func;

/* Program loader */
uint8_t load_program(uint8_t * file_name, uint8_t * location, uint32_t * eip);

#endif // ASM
#endif // _FILE_SYSTEM_H
