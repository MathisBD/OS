CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -D__is_kernel -Iinclude
LIBS:=$(LIBS) -nostdlib -lgcc


OBJS=kernel.o boot.o \
tables/idt.o tables/idt_asm.o\
tables/gdt.o tables/gdt_asm.o\
drivers/vga_driver.o \
drivers/pic_driver.o \
drivers/io_asm.o \
drivers/keyboard_driver.o \
drivers/pit_driver.o \
utils/string_utils.o \
memory/paging.o \
memory/memory_asm.o \
memory/bitset.o \
memory/heap.o \
scheduler/timer.o 

.PHONY: all clean install install-headers install-kernel

all: minios.kernel

minios.kernel: $(OBJS) linker.o
	$(CC) -T linker.o $(CFLAGS) -nostdlib -o $@ $(OBJS) $(LIBS)
	grub-file --is-x86-multiboot minios.kernel

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


install: install-headers install-kernel

install-headers:
	mkdir -p $(SYSROOT)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(SYSROOT)$(INCLUDEDIR)/.

install-kernel: minios.kernel
	mkdir -p $(SYSROOT)$(BOOTDIR)
	cp minios.kernel $(SYSROOT)$(BOOTDIR)


clean:
	rm -f $(OBJS)
	rm -f linker.o
	rm -f minios.kernel
	rm -f $(OBJS:.o=.d)

-include $(OBJS:.o=.d)