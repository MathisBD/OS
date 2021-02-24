
// CAREFUL : needs to work even if 
// src and dest overlap
.global memmove
memmove:
    mov 4(%esp), %edi  // dest
    mov 8(%esp), %esi  // src 
    mov 12(%esp), %ecx // size
    mov %edi, %eax // return value

    cmp %edi, %esi 
    jl 1f

    // forwards
    cld
    rep movsb
    ret 
1:
    // backwards
    lea -1(%edi, %ecx), %edi
    lea -1(%esi, %ecx), %esi
    std
    rep movsb 
    ret