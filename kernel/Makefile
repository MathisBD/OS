CFLAGS?=-O2 -g
CPPFLAGS?=
LIBS?=

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -D__is_kernel \
-I$(SYSROOT)$(INCLUDEDIR)/kernel \
-I$(SYSROOT)$(INCLUDEDIR)/libc

LIBS:=$(LIBS) -nostdlib -lgcc -lk

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
memory/heap.o \
scheduler/timer.o \
loader/loader.o \
loader/loader_asm.o \
filesystem/ext2/ext2_gen.o \
filesystem/ext2/ext2_rw.o \
filesystem/ext2/ext2_dir.o \
filesystem/ext2/ext2_alloc.o

.PHONY: all clean install install-headers install-kernel

all: $(KERNEL)

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
install: install-headers install-kernel

install-headers:
	mkdir -p $(SYSROOT)$(INCLUDEDIR)/kernel
	cp -R --preserve=timestamps include/. $(SYSROOT)$(INCLUDEDIR)/kernel/.

install-kernel: $(KERNEL)
	mkdir -p $(SYSROOT)$(BOOTDIR)
	cp $(KERNEL) $(SYSROOT)$(BOOTDIR)

clean:
	rm -f ./*.o ./*/*.o ./*/*/*.o
	rm -f ./*.d ./*/*.d ./*/*/*.d
	rm -f ./*.elf ./*/*.elf ./*/*/*.elf
	rm -f ./*.bin ./*/*.bin ./*/*/*.bin

-include $(KERNEL_OBJS:.o=.d)
