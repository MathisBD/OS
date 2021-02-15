.global load_gdtr

// arguments (stack) :
//   1) pointer to the new contents of gdtr
load_gdtr:
    // load gdtr
    mov 4(%esp), %eax
    lgdt (%eax) 

    // reload every segment register
    // 0x10 is the byte offset of the kernel 
    // data segment descriptor in the gdt
    mov $0x10, %ax 
    mov %ax, %ds 
    mov %ax, %es 
    mov %ax, %fs 
    mov %ax, %gs 
    mov %ax, %ss
    // a far jump implicitly reloads %cs,
    // here using 0x08 (offset of the kernel code descriptor)
    // TODO : find the right syntax jmp 0x08:.finish
.finish:
    ret
