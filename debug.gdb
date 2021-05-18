shell ./image.sh

add-symbol-file kernel/minios.elf
add-symbol-file progs/shell.elf

target remote | qemu-system-i386 -S -gdb stdio \
-drive file=hdd_minios.img,format=raw,index=0,media=disk \
-m 512 -no-reboot -no-shutdown

// run with 'gdb -x debug/gdb'