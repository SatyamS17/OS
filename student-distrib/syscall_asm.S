#define ASM 1

.globl syscall_handler
syscall_handler:
    # Push all registers except for eax, since eax should be set to the handler's return value and not saved.
    pushl   %gs
    pushl   %fs
    pushl   %es
    pushl   %ds
    pushl   %ebp
    pushl   %esi
    pushl   %edx
    pushl   %ecx
    pushl   %ebx

    pushl   %edx
    pushl   %ecx
    pushl   %ebx

    cmpl    $10, %eax                   # check %eax <= 10
    jg      syscall_handler_err
    cmpl    $1, %eax                    # check %eax >= 1
    jl      syscall_handler_err

    decl    %eax                        # decrement %eax to be zero-indexed for the jump table
    call    *syscall_handler_jumptable(, %eax, 4)

    jmp     syscall_handler_ret

syscall_handler_err:
    movl    $-1, %eax
    jmp     syscall_handler_ret

syscall_handler_ret:
    addl    $12, %esp                   # pop args
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

.globl flush_tlb
flush_tlb:
    movl	%cr3, %eax
    movl	%eax, %cr3
    ret
