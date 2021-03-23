Built a gcc cross compiler (i686-elf-gcc) and binutils (e.g. for ld).
Used tutorial Bare Bones from OSdev to get a basic kernel working.
Used some code from duneOS for setting up paging in assembly :  
https://github.com/naegelejd/duneOS/blob/master/kernel/start.s

Bochs io port list : http://bochs.sourceforge.net/techspec/PORTS.LST
ATA pio driver example : https://github.com/mit-pdos/xv6-public/blob/master/ide.c#L55
Useful tutorials/references : 
General :
http://www.brokenthorn.com/Resources/OSDevIndex.html
http://www.jamesmolloy.co.uk/tutorial_html/
Bootloader : (heavily inspired, except they use FAT and I use EXT2)
http://independent-software.com/operating-system-development-first-and-second-stage-bootloaders.html/ 
BIOS interrupts :
http://www.ablmcc.edu.hk/~scy/CIT/8086_bios_and_dos_interrupts.htm#int10h_0Eh
Ext2 :
https://wiki.osdev.org/Ext2
https://www.nongnu.org/ext2-doc/ext2.html#directory


We use the compiler option --sysroot=sdir so that gcc searches for headers and libraries in sdir/usr/include and sdir/usr/lib instead of /usr/include and /usr/lib. Thus we have a script (header.sh) to copy all headers into sysroot/usr/include.
The operating system will be 32bits to simplify stuff. The kernel is mapped to 3GB in virtual address space.
The kernel uses the EXT2 filesystem, revision 0.