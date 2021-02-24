CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -Iinclude
LIBS:=$(LIBS) -nostdlib -lgcc


.PHONY: all clean install install-headers install-user

OBJS=test.o

all: user.o

user.o: $(OBJS) linker.ld
	$(CC) -T linker.ld $(CFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) -MD -c $< -o $@ -std=gnu99 $(CFLAGS) $(CPPFLAGS)

# we run the C preprocessor on assembly files
# (implicit since gcc does it on .S files)
%.o: %.S
	$(CC) -MD -c $< -o $@ -std=gnu99 $(CFLAGS) $(CPPFLAGS)

install: install-headers install-user

install-headers:
	mkdir -p $(SYSROOT)$(INCLUDEDIR)/user
	cp -R --preserve=timestamps include/. $(SYSROOT)$(INCLUDEDIR)/user/.

install-user: user.o
	mkdir -p $(SYSROOT)$(BOOTDIR)
	cp --preserve=timestamps user.o $(SYSROOT)$(BOOTDIR)

clean:
	rm -f $(OBJS)
	rm -f $(OBJS:.o=.d)

-include $(OBJS:.o=.d)