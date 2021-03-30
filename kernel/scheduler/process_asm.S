
// returns the value of esp previous to this function call
.global get_esp
get_esp:
    mov %esp, %eax 
    add $4, %eax 
    ret 
