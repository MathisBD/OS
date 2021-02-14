CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -D__is_kernel -Iinclude
LIBS:=$(LIBS) -nostdlib -lk -lgcc

#OBJS=\
kernel.o boot.o \
idt.o idt_asm.o \
drivers/io_asm.o \
drivers/keyboard_driver_asm.o drivers/keyboard_driver.o \
drivers/pic_driver.o \
drivers/vga_driver.o

OBJS=kernel.o boot.o \
idt.o idt_asm.o\
drivers/vga_driver.o \
drivers/pic_driver.o \
drivers/io_asm.o \
drivers/keyboard_driver.o drivers/keyboard_driver_asm.o

LINK_LIST=\
$(OBJS)

.PHONY: all clean install install-headers install-kernel

all: minios.kernel

minios.kernel: $(OBJS) linker.ld
	$(CC) -T linker.ld $(CFLAGS) -nostdlib -o $@ $(LINK_LIST)

%.o: %.c
	$(CC) -MD -c $< -o $@ -std=gnu99 $(CFLAGS) $(CPPFLAGS)

%.o: %.s
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
	rm -f minios.kernel
	rm -f $(OBJS:.o=.d)

-include $(OBJS:.o=.d)