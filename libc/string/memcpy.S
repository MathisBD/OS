
.global memcpy
memcpy:
    mov 4(%esp), %edi  // dest
    mov 8(%esp), %esi  // src
    mov 12(%esp), %ecx // size
    mov %edi, %eax // return value
    cld
    rep movsb
    ret
