#include "paging.h"

/* 
 * init_preg
 *   DESCRIPTION: Enables paging by setting appropriate bits in CR0, CR3, adn CR4.
 *          
 *   INPUTS: int page_dir address
 *   OUTPUTS: none
 *   RETURN VALUE: void
 *   SIDE EFFECTS: Enables paging
 *  
 */

extern void init_preg(int);

/* 
 * paging_init
 *   DESCRIPTION: Intializes paging by setting up page directory and table along with
 *              video memory and kernel. Also enables paging by setting appropriate bits.
 *          
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: void
 *   SIDE EFFECTS: Intializes paging (video mem and kernel)
 *  
 */

extern void paging_init()
{
    int i;

    // Set default not-present page entries for the directory and table
    for (i = 0; i < PAGE_NUM; i++)
    {
        // init the page directory
        page_dir[i].present = 0;
        page_dir[i].read_write = 1;
        page_dir[i].user_supervisor = 0;
        page_dir[i].write_through = 0;
        page_dir[i].cache_disabled = 0;
        page_dir[i].accessed = 0;
        page_dir[i].reserved = 0;
        page_dir[i].page_size = 1;
        page_dir[i].global_page = 0;
        page_dir[i].available = 0;

        // init page table
        page_table[i].present = 0;
        page_table[i].read_write = 1;
        page_table[i].user_supervisor = 0;
        page_table[i].write_through = 0;
        page_table[i].cache_disabled = 0;
        page_table[i].accessed = 0;
        page_table[i].dirty = 0;
        page_table[i].page_attribute = 0;
        page_table[i].global_page = 0;
        page_table[i].available = 0;
        page_table[i].base_address = i;
    }

    // init video memory (4KB) by setting it as present in both the table and directory
    page_dir[0].present = 1;
    page_dir[0].page_size = 0;
    page_dir[0].page_table_address = ((int)page_table) / FOURKB_BITS;
    page_table[VID_MEM_INDEX].present = 1;
    
    // init kernel by making it present and looking at kenerl address
    page_dir[1].present = 1;
    page_dir[1].page_table_address = ((int)KERNEL_ADDRESS) / FOURKB_BITS;

    /* updating registers to init paging (CRO, CR3, CR4)*/
    init_preg((int)page_dir);
}
