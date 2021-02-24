.global memset
memset:
    mov 4(%esp), %edi  // dest
    mov 8(%esp), %eax  // c
    mov 12(%esp), %ecx // count
    mov %edi, %edx // return value
    cld // clear direction flag
    rep stosb
    mov %edx, %eax
    ret
