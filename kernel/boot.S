#include "memory/constants.h"

.extern kernel_main
.extern get_mmap
.global _start

// GRUB constants
.set MB_MAGIC, 0x1BADB002
// load modules on page boundaries + provide memory map
.set MB_FLAGS, (1 << 0) | (1 << 1) 
.set MB_CHECKSUM, (0 - (MB_MAGIC + MB_FLAGS))


.section .text

// multiboot header
    .align 4 
    .long MB_MAGIC
    .long MB_FLAGS
    .long MB_CHECKSUM

// multiboot header
//.set MH_MAGIC, 0xE85250D6
//.set MH_ARCH, 0
//.set MH_LEN, (mb_end - mb_start)
//.align 4
//mb_start:
//.long MH_MAGIC         // magic 
//.long MH_ARCH           // architecture
//.long MH_LEN              // header length
//.long - (MH_MAGIC + MH_LEN + MH_ARCH) // checksum 

// TAGS
// 
//mb_end:

.section .bss
// stack space
.align 16
stack_bottom:
    .skip STACK_SIZE
stack_top:

.section .data
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
_start:
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
    mov %ecx, %cr3 // start the kernel

    // initialize stack
    mov $stack_top, %esp 
    
    cli
    
    // GRUB puts some values in %ebx (multiboot header address) and %eax (multiboot magic)
    push %eax
    add $V_KERNEL_START, %ebx 
    push %ebx
    call kernel_main   
    add $8, %esp  

    // this should not happen
    cli
    hlt

