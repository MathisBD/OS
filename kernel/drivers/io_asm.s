.global read_io_port
.global write_io_port

// arguments (stack) :
//   1) IO port number
// returns (eax) :
//   byte read from the port
read_io_port:
    mov 4(%esp), %edx
    in %dx, %al
    ret 

// arguments (stack) :
//   1) IO port number
//   2) byte to write 
write_io_port:
    mov 4(%esp), %edx
    mov 8(%esp), %eax 
    out %al, %dx
    ret
    