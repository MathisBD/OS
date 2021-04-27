# We compile files twice :
# once into .libc.o (used to create libc.a)
# and once into .libk.o (used to create libk.a)
# each time with different options of course

CFLAGS?=-O2 -g
CPPFLAGS?=
LIBS?=

LIBK_CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
LIBK_CPPFLAGS:=$(CPPFLAGS) -D__is_libk \
-I$(SYSROOT)$(INCLUDEDIR)/libc \
-I$(SYSROOT)$(INCLUDEDIR)/kernel

LIBC_CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
LIBC_CPPFLAGS:=$(CPPFLAGS) -D__is_libc \
-I$(SYSROOT)$(INCLUDEDIR)/libc

LIBK_OBJS= \
stdio/printf.libk.o \
stdio/putchar.libk.o \
stdio/puts.libk.o \
stdio/print_mem.libk.o \
string/memcmp.libk.o \
string/memcpy.libk.o \
string/memset.libk.o \
string/memmove.libk.o \
string/strlen.libk.o \
panic/panic.libk.o \
stdlib/div.libk.o \
bitset/bitset_create.libk.o \
bitset/bitset_set.libk.o \
bitset/bitset_clear.libk.o \
bitset/bitset_find_one.libk.o \
bitset/bitset_find_zero.libk.o \
bitset/bitset_test.libk.o \
syscall/exit.libk.o \
syscall/new_thread.libk.o \
list/list_create.libk.o \
list/list_delete.libk.o \
list/list_empty.libk.o \
list/list_add_front.libk.o \
list/list_add_back.libk.o \
list/list_pop_front.libk.o \
list/list_pop_back.libk.o

LIBC_OBJS=

# BINARIES=libk.a libc.a
BINARIES=libk.a

.PHONY: all clean install install-headers install-libs

all: $(BINARIES)

# LIBC
libc.a: $(LIBC_OBJS) 
	$(AR) rcs $@ $(LIBC_OBJS)

%.libc.o: %.c 
	$(CC) -MD -c $< -o $@ -std=gnu99 $(LIBC_CFLAGS) $(LIBC_CPPFLAGS)

%.libc.o: %.S 
	$(CC) -MD -c $< -o $@ -std=gnu99 $(LIBC_CFLAGS) $(LIBC_CPPFLAGS)

# LIBK
libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)

%.libk.o: %.c 
	$(CC) -MD -c $< -o $@ -std=gnu99 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

%.libk.o: %.S 
	$(CC) -MD -c $< -o $@ -std=gnu99 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)


install: install-headers install-libs 

install-headers:
	mkdir -p $(SYSROOT)$(INCLUDEDIR)/libc 
	cp -R --preserve=timestamps include/. $(SYSROOT)$(INCLUDEDIR)/libc/.

install-libs: $(BINARIES)
	mkdir -p $(SYSROOT)$(LIBDIR)
	cp $(BINARIES) $(SYSROOT)$(LIBDIR)

clean:
	rm -f $(BINARIES)
	rm -f $(LIBC_OBJS) $(LIBK_OBJS)
	rm -f $(LIBC_OBJS:.o=.d) $(LIBK_OBJS:.o=.d)

-include $(LIBK_OBJS:.o=.d) 
#-include $(LIBC_OBJS:.o=.d)