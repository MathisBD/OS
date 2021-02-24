
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
