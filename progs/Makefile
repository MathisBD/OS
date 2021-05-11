CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -O0
CPPFLAGS:=$(CPPFLAGS) \
-I$(SYSROOT)$(INCLUDEDIR)/libc

LIBS:=$(LIBS) -nostdlib -lc -lgcc 


.PHONY: all clean install install-headers install-user

OBJS= \
test.o \
echo.o

#PROGS=$(basename $(OBJS))
PROGS=$(OBJS:.o=.elf)

all: $(PROGS)

%.elf: %.o linker.ld $(SYSROOT)$(LIBDIR)/libc.a
	$(CC) -T linker.ld $(CFLAGS) $< -o $@ $(LIBS)

%.o: %.c
	$(CC) -MD -c $< -o $@ -std=gnu99 $(CFLAGS) $(CPPFLAGS)

# we run the C preprocessor on assembly files
# (implicit since gcc does it on .S files)
%.o: %.S
	$(CC) -MD -c $< -o $@ -std=gnu99 $(CFLAGS) $(CPPFLAGS)

install: install-progs

install-progs: $(PROGS)
	@mkdir -p $(SYSROOT)/progs
	@cp --preserve=timestamps $(PROGS) $(SYSROOT)/progs

clean:
	@rm -f $(OBJS)
	@rm -f $(OBJS:.o=.d)
	@rm -f $(PROGS)

-include $(OBJS:.o=.d)