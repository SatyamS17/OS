#define ASM     1 

.global init_preg

init_preg:
    /* setup stack*/
    pushl %ebp
    movl %esp, %ebp

    /* load page directory address into cr3 */
    movl 8(%ebp), %eax
    movl %eax, %cr3

    /* set PSE bit in cr4 */
    movl %cr4, %eax
    orl  $0x00000010, %eax
    movl %eax, %cr4

    /* set PG AND PE bit in cr0*/
    movl %cr0, %eax
    orl  $0x80000001, %eax
    movl %eax, %cr0

    /* stack teardown */
    leave
    ret
