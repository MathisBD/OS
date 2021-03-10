CFLAGS?=-O2 -g
CPPFLAGS?=
LIBS?=

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -D__is_bootloader \
-I$(SYSROOT)$(INCLUDEDIR)/bootloader

# no libk for the bootloader
LIBS:=$(LIBS) -nostdlib -lgcc


BOOT_SRCS=asm_code/first_stage.S \
asm_code/second_stage.S \
asm_code/functions.S \
asm_code/macros.S \
asm_code/functions_extra.S \
asm_code/a20.S

# only for files that will be compiled and linked
# in the stage 2
BOOT_S2_OBJS=c_code/s2_main.o \
c_code/vga_minidriver.o \
c_code/ata_minidriver.o \
c_code/miniloader.o \
asm_code/pio.o \
asm_code/miniloader_asm.o

.PHONY: all clean install install-bootloader install-headers

all: $(FIRST_STAGE) $(SECOND_STAGE)

$(FIRST_STAGE): $(BOOT_SRCS)
	$(CC) -c asm_code/first_stage.S -o asm_code/first_stage.o
	# extract the .text section
	objcopy -O binary -j .text asm_code/first_stage.o tmp.bin
	# remove the first 0x7c00=62*512 first bytes
	# the first stage is always exactly 512 bytes long 
	dd if=./tmp.bin of=./$(FIRST_STAGE) bs=512 count=1 skip=62
	rm tmp.bin
	
$(SECOND_STAGE): $(BOOT_SRCS) $(BOOT_S2_OBJS) s2_linker.ld
	# compile assembly files 
	$(CC) -c asm_code/second_stage.S -o asm_code/second_stage.o
	# link 
	$(CC) -T s2_linker.ld $(CFLAGS) -o tmp.o \
	$(BOOT_S2_OBJS) asm_code/second_stage.o $(LIBS)
	# extract the .text section
	objcopy -O binary -j .text tmp.o tmp.bin
	# remove the first 0x8000=1024*32 first bytes
	# the second stage is always 12KB bytes long
	dd if=./tmp.bin of=./$(SECOND_STAGE) bs=1024 count=12 skip=32
	truncate -s 12K $(SECOND_STAGE)
	rm tmp.bin

%.o: %.c
	$(CC) -MD -c $< -o $@ -std=gnu99 $(CFLAGS) $(CPPFLAGS)

%.o: %.S
	$(CC) -MD -c $< -o $@ -std=gnu99 $(CFLAGS) $(CPPFLAGS)

install: install-bootloader install-headers

install-headers:
	mkdir -p $(SYSROOT)$(INCLUDEDIR)/bootloader
	cp -R --preserve=timestamps include/. $(SYSROOT)$(INCLUDEDIR)/bootloader/.

install-bootloader: $(FIRST_STAGE) $(SECOND_STAGE)
	mkdir -p $(SYSROOT)$(BOOTDIR)
	cp $(FIRST_STAGE) $(SYSROOT)$(BOOTDIR)
	cp $(SECOND_STAGE) $(SYSROOT)$(BOOTDIR)

clean: 
	rm -f ./*.o ./*/*.o ./*/*/*.o
	rm -f ./*.d ./*/*.d ./*/*/*.d
	rm -f ./*.elf ./*/*.elf ./*/*/*.elf
	rm -f ./*.bin ./*/*.bin ./*/*/*.bin

-include $(BOOT_S2_OBJS:.o=.d)