# 1 "syscall/new_thread.S"
# 1 "/home/mathis/src/OS/libc//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "syscall/new_thread.S"



.macro sc_arg_1
    %ebx
.endm
.macro sc_arg_2
    %ecx
.endm
.macro sc_arg_3
    %edx
.endm
.macro sc_arg_4
    %edi
.endm
.macro sc_arg_5
    %esi
.endm


.global new_thread
new_thread:


    .extern kmalloc
    push $8
    call kmalloc





    mov 4(%esp), %ecx
    mov %ecx, (%eax)

    mov 8(%esp), %ecx
    mov %ecx, 4(%esp)

    mov %eax, (%eax)

    mov $thread_wrapper, sc_arg_1
    mov %eax, sc_arg_2
    mov 12(%esp), sc_arg_3
    mov 16(%esp), sc_arg_4
    int $0x80
    ret


.extern exit


thread_wrapper:
    mov (%eax), %ebx
    mov 4(%eax), %ecx
    push %ecx
    call *%ebx


    call exit
