CFLAGS?=-O2 -g
CPPFLAGS?=
LIBS?=

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -D__is_kernel \
-I$(SYSROOT)$(INCLUDEDIR)/kernel \
-I$(SYSROOT)$(INCLUDEDIR)/libc

LIBS:=$(LIBS) -nostdlib -lgcc -lk

BOOT_SRCS=bootloader/first_stage.S \
bootloader/second_stage.S \
bootloader/functions.S \
bootloader/macros.S \

KERNEL_OBJS=kernel.o boot.o \
tables/idt.o tables/idt_asm.o \
tables/gdt.o tables/gdt_asm.o \
drivers/vga_driver.o \
drivers/pic_driver.o \
drivers/io_asm.o \
drivers/keyboard_driver.o \
drivers/pit_driver.o \
drivers/ata_driver.o \
utils/string_utils.o \
memory/paging.o \
memory/memory_asm.o \
memory/bitset.o \
memory/heap.o \
scheduler/timer.o \
loader/loader.o \
loader/loader_asm.o

.PHONY: all clean install install-headers install-kernel

all: $(KERNEL) $(FIRST_STAGE) $(SECOND_STAGE)

$(FIRST_STAGE): $(BOOT_SRCS)
	$(CC) -c bootloader/first_stage.S -o bootloader/first_stage.o
	# extract the .text section
	objcopy -O binary -j .text bootloader/first_stage.o tmp.elf
	# remove the first 0x7c00=62*512 first bytes
	# the first stage is always exactly 512 bytes long 
	dd if=./tmp.elf of=./$(FIRST_STAGE) bs=512 count=1 skip=62
	rm tmp.elf
	
$(SECOND_STAGE): $(BOOT_SRCS)
	$(CC) -c bootloader/second_stage.S -o bootloader/second_stage.o
	# extract the .text section 
	objcopy -O binary -j .text bootloader/second_stage.o tmp.elf 
	# remove the first 0x8000=1024*32 first bytes
	# the second stage is always 12KB bytes long
	dd if=./tmp.elf of=./$(SECOND_STAGE) bs=1024 count=12 skip=32
	rm tmp.elf

# kind of a hack to add libk.a in the dependencies
$(KERNEL): $(KERNEL_OBJS) linker.o	$(SYSROOT)$(LIBDIR)/libk.a
	$(CC) -T linker.o $(CFLAGS) -o $@ $(KERNEL_OBJS) $(LIBS)

# we run the C preprocessor on the linker script
%.o: %.ld
	# grep to remove comments (in particular #include lines)
	$(CC) -x c -E $(CPPFLAGS) $< | grep -v '^#' >$@

%.o: %.c
	$(CC) -MD -c $< -o $@ -std=gnu99 $(CFLAGS) $(CPPFLAGS)

# we run the C preprocessor on assembly files
# (implicit since gcc does it on .S files)
%.o: %.S
	$(CC) -MD -c $< -o $@ -std=gnu99 $(CFLAGS) $(CPPFLAGS)


# install to the sysroot
install: install-headers install-boot

install-headers:
	mkdir -p $(SYSROOT)$(INCLUDEDIR)/kernel
	cp -R --preserve=timestamps include/. $(SYSROOT)$(INCLUDEDIR)/kernel/.

install-boot: $(FIRST_STAGE) $(SECOND_STAGE) $(KERNEL)
	mkdir -p $(SYSROOT)$(BOOTDIR)
	cp $(FIRST_STAGE) $(SYSROOT)$(BOOTDIR)
	cp $(SECOND_STAGE) $(SYSROOT)$(BOOTDIR)
	cp $(KERNEL) $(SYSROOT)$(BOOTDIR)

clean:
	rm -f ./*.o ./*/*.o ./*/*/*.o
	rm -f ./*.d ./*/*.d ./*/*/*.d
	rm -f ./*.elf ./*/*.elf ./*/*/*.elf

-include $(KERNEL_OBJS:.o=.d)