#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#ifndef ASM

#include "types.h"

#define BLOCK_SIZE              4096
#define DATA_BLOCK_NUM          1023
#define FILENAME_SIZE           32
#define DIR_ENTRY_RESERVED      24
#define BOOT_BLOCK_RESERVED     52
#define DIR_ENTRIES_NUM         63

/* data block struct */
typedef struct data_block_t
{
    uint8_t data[BLOCK_SIZE];
} data_block_t;

/* inode struct */
typedef struct inodes_t
{
    uint32_t length;
    uint32_t data_block[DATA_BLOCK_NUM];
} inodes_t;

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

/* Intialize file system*/
boot_block_t * file_system;
void file_system_init(uint32_t * address); // MAKE SURE TO ADD TO KERNEL.C!!

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

/* Test functions */
void d_read_test();
void f_read_test(uint8_t * test_name);
void f_read_long_test();
void f_read_short_test();
void f_read_exec_test();
void f_read_noread_test();
void test_read_dentry_index();
void test_read_dentry_name();

#endif // ASM
#endif // _FILE_SYSTEM_H
