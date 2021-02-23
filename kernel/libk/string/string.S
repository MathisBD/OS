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

.global memcpy
memcpy:
    mov 4(%esp), %edi  // dest
    mov 8(%esp), %esi  // src
    mov 12(%esp), %ecx // size
    mov %edi, %eax // return value
    cld
    rep movsb
    ret

.global memcmp
memcmp:
    mov 4(%esp), %edi  // ptr_1
    mov 8(%esp), %esi  // ptr_2
    mov 12(%esp), %ecx // count
    cld 

    // special case : count == 0
    test %ecx, %ecx
    repe cmpsb
    jnz 1f

    // equal 
    mov $0, %eax 
    ret 
1:
    // not equal
    dec %edi 
    dec %esi 
    mov (%edi), %eax 
    sub (%esi), %eax 
    ret

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