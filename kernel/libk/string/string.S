.global memset
memset:
    mov 4(%esp), %edi  // dest
    mov 8(%esp), %eax  // c
    mov 12(%esp), %ecx // size
    rep stosb
    ret

    