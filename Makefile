all: kernel

kernel: start.o kernel.o linker.ld
	i686-elf-gcc -ffreestanding -nostdlib -g -T linker.ld start.o kernel.o -o myos.bin -lgcc -O2

start.o: start.s
	i686-elf-gcc -std=gnu99 -ffreestanding -g -c start.s -o start.o

kernel.o: kernel.c
	i686-elf-gcc -std=gnu99 -ffreestanding -g -c kernel.c -o kernel.o
	
clean:
	rm ./*.o