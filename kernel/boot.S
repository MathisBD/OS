#include "memory/constants.h"

.extern kernel_main
.global init_stack_bottom
.global init_stack_top


.global phys_entry
.set phys_entry, (entry - V_KERNEL_START)  

.section .bss
// stack space
.align 16
init_stack_bottom:
    .skip KSTACK_SIZE
init_stack_top:

.section .data
boot_info_addr: .int 0

// boot page directory
.set KERNEL_PAGE_IDX, V_KERNEL_START >> 22
.align 0x1000 // page directories must be aligned on 4K
boot_page_dir:
    // bits :
    // 0 : present 
    // 1 : read/write 
    // 7 : PS (4MB pages)
    .int 0x00000083 // first 4MB
    .fill (KERNEL_PAGE_IDX - 1), 4, 0x00000000
    .int 0x00000083 // kernel 4MB page
    .fill (PD_SIZE - KERNEL_PAGE_IDX - 1), 0x00000000

.section .text
entry:
    // save the boot info that the bootloader passes us 
    add $V_KERNEL_START, %eax 
    // virtual memory isn't enabled yet at this point
    mov %eax, (boot_info_addr - V_KERNEL_START)

    // load the page directory 
    mov $(boot_page_dir - V_KERNEL_START), %ecx 
    mov %ecx, %cr3

    // enable 4MB pages 
    mov %cr4, %ecx
    or 0x00000010, %ecx
    mov %ecx, %cr4

    // enable paging (set the paging bit in cr0)
    mov %cr0, %ecx
    or $0x80000000, %ecx
    mov %ecx, %cr0

    // jump to higher half with an absolute jump
    lea launch_kernel, %ecx 
    jmp *%ecx

.section .text
launch_kernel:
    // unmap the first page 
    movl $0, boot_page_dir
    invlpg 0

    // reload cr3 to force the changes to take effect
    mov %cr3, %ecx 
    mov %ecx, %cr3

    // initialize stack
    mov $init_stack_top, %esp 
    
    cli
    push boot_info_addr
    call kernel_main   

    // this should not happen
    cli
    hlt

