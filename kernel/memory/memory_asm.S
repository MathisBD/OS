.macro get_cr n
.global get_cr\n
get_cr\n:
    mov %cr\n, %eax
    ret
.endm

.macro load_cr n 
.global load_cr\n 
load_cr\n:
    mov 4(%esp), %eax 
    mov %eax, %cr\n
    ret
.endm 

get_cr 0
get_cr 1
get_cr 2
get_cr 3
get_cr 4

load_cr 0
load_cr 1
load_cr 2
load_cr 3
load_cr 4