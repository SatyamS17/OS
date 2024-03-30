#define ASM 1

.globl syscall_handler
syscall_handler:
    pushl   %gs
    pushl   %fs
    pushl   %es
    pushl   %ds
    pushl   %ebp
    pushl   %esi
    pushl   %edx
    pushl   %ecx
    pushl   %ebx
    pushl   %eax

    pushl   %edx
    pushl   %ecx
    pushl   %ebx

    cmpl    $10, %eax                   # check %eax <= 10
    jg      syscall_handler_ret
    cmpl    $1, %eax                    # check %eax >= 1
    jl      syscall_handler_ret

    decl    %eax                        # decrement %eax to be zero-indexed for the jump table
    jmp     *syscall_handler_jumptable(, %eax, 4)
    incl    %eax

    jmp     syscall_handler_ret

syscall_handler_ret:
    addl    $12, %esp                   # pop args
    popl    %eax
    popl    %ebx
    popl    %ecx
    popl    %edx
    popl    %esi
    popl    %ebp
    popl    %ds
    popl    %es
    popl    %fs
    popl    %gs
    iret

syscall_handler_jumptable:
    .long   halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
