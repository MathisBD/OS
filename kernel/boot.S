#include "constants.h"

.extern kernel_main
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


.section .bss
// stack space
.align 16
stack_bottom:
    .skip STACK_SIZE
stack_top:

.section .data
// boot page directory
.set KERNEL_PAGE_IDX, V_KERNEL_START >> 22
.align PAGE_SIZE
boot_page_dir:
    // bits :
    // 0 : present 
    // 1 : read/write 
    // 7 : PS (4MB pages)
    .int 0x00000083 // first 4MB
    .fill (KERNEL_PAGE_IDX - 1), 4, 0x00000000
    .int 0x00000083 // kernel 4MB page
    .fill (1024 - KERNEL_PAGE_IDX - 1), 0x00000000

//.section .text
//_start:
    // ecx holds the number of pages left to map
    //mov $(PT_SIZE - 1), %ecx
    // esi holds the current physical address
    //mov 0, %esi
    // edi holds the physical address of the current entry in the page table
    //mov $(page_table_1 - V_KERNEL_START), %edi

// map kernel portion
//map:
    // finished mapping ?
    //cmp $(_kernel_rw_end - V_KERNEL_START), %esi
    //jge finish_mapping
    // need to map current address ?
    //cmp $(_kernel_start - V_KERNEL_START), %esi
    //jl 2f

    // map current address
    // set the page as "present" if read-only, 
    // and "present, writeable" if read-write
    //mov $0x01, %edx
    //mov $0x03, %ebx
    //cmp $(_kernel_ro_end - V_KERNEL_START), %esi
    //cmovge %ebx, %edx

    //or %esi, %edx
    //mov %edx, (%edi)

//2:
    // increase stuff
    //add $4, %edi // page table entry is 4 bytes long
    //add $PAGE_SIZE, %esi
    //loop map

// finished mapping
//finish_mapping:
    // map the VGA buffer
    //movl $(P_VGA_ADDRESS + 0x03), page_table_1 - V_KERNEL_START + 1023 * 4

    // map the page tables as "present, writeable"
    // 768 is V_KERNEL_START / (PAGE_SIZE * PT_SIZE)
    //movl $(page_table_1 - V_KERNEL_START + 0x03), page_dir - V_KERNEL_START + 0
    //movl $(page_table_1 - V_KERNEL_START + 0x03), page_dir - V_KERNEL_START + 768 * 4

    // load the physical address of the page dir into cr3
    //mov $(page_dir - V_KERNEL_START), %ecx 
    //mov %ecx, %cr3

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
    
    // GRUB puts some values in %ebx (multiboot header address) and %eax (multiboot magic)
    push %eax
    add $V_KERNEL_START, %ebx 
    push %ebx

    // start the kernel 
    cli
    call kernel_main

    // this should not happen
    cli
    hlt

