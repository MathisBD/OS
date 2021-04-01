# 1 "syscall/exit.S"
# 1 "/home/mathis/src/OS/libc//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "syscall/exit.S"
# 1 "/home/mathis/src/OS/sysroot/usr/include/libc/__syscall_internal.h" 1
# 2 "syscall/exit.S" 2

.global exit
exit:
    mov 3, SC_ARC_0
    mov 4(%esp), %ebx
    int $0x80
    ret
