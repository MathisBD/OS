.global memset
memset:
    mov 4(%esp), %edi  // dest
    mov 8(%esp), %eax  // c
    mov 12(%esp), %ecx // size
    rep stosb
    ret


.macro get_cr n
.global get_cr\n
get_cr\n:
    mov %cr\n, %eax
    ret
.endm

get_cr 0
get_cr 1
get_cr 2
get_cr 3
get_cr 4