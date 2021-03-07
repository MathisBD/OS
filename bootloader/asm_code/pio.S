.global port_int8_in
.global port_block_in

.global port_int8_out
.global port_int16_out
.global port_int32_out
.global port_block_out

// arguments (stack) :
//   1) IO port number
// returns (eax) :
//   data read from the port
port_int8_in:
    mov 4(%esp), %edx
    in %dx, %al
    ret 

// arguments (stack) :
//   1) IO port number 
//   2) address to read to 
//   3) number of uint32 to read
port_block_in:
    mov 4(%esp), %edx 
    mov 8(%esp), %edi
    mov 12(%esp), %ecx 
    cld
    rep insl
    ret

// arguments (stack) :
//   1) IO port number
//   2) data to write 
port_int8_out:
    mov 4(%esp), %edx
    mov 8(%esp), %eax 
    out %al, %dx
    ret

port_int16_out:
    mov 4(%esp), %edx
    mov 8(%esp), %eax 
    out %ax, %dx
    ret

port_int32_out:
    mov 4(%esp), %edx 
    mov 8(%esp), %eax 
    out %eax, %dx 
    ret

// arguments :
//   1) IO port number 
//   2) address of data to write
//   3) number of uint32 to write
port_block_out:
    mov 4(%esp), %edx 
    mov 8(%esp), %esi 
    mov 12(%esp), %ecx 
    cld 
    rep outsl
    ret