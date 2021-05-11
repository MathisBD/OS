# We compile files twice :
# once into .libc.o (used to create libc.a)
# and once into .libk.o (used to create libk.a)
# each time with different options of course

CFLAGS?=-g
CPPFLAGS?=
LIBS?=

LIBK_CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -O0
LIBK_CPPFLAGS:=$(CPPFLAGS) -D__is_libk \
-I$(SYSROOT)$(INCLUDEDIR)/libc \
-I$(SYSROOT)$(INCLUDEDIR)/kernel \
-Iinclude_internal

LIBC_CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
LIBC_CPPFLAGS:=$(CPPFLAGS) -D__is_libc \
-I$(SYSROOT)$(INCLUDEDIR)/libc \
-Iinclude_internal

LIBK_OBJS= \
stdio/printf.libk.o \
stdio/putchar.libk.o \
stdio/fputchar.libk.o \
stdio/fprintf.libk.o \
stdio/print_mem.libk.o \
string/memcmp.libk.o \
string/memcpy.libk.o \
string/memset.libk.o \
string/memmove.libk.o \
string/strlen.libk.o \
panic/panic.libk.o \
stdlib/div.libk.o \
stdlib/itoa.libk.o \
bitset/bitset_create.libk.o \
bitset/bitset_set.libk.o \
bitset/bitset_clear.libk.o \
bitset/bitset_find_one.libk.o \
bitset/bitset_find_zero.libk.o \
bitset/bitset_test.libk.o \
list/list_create.libk.o \
list/list_delete.libk.o \
list/list_empty.libk.o \
list/list_add_front.libk.o \
list/list_add_back.libk.o \
list/list_pop_front.libk.o \
list/list_pop_back.libk.o \
list/list_remove_node.libk.o \
blocking_queue/bq_create.libk.o \
blocking_queue/bq_delete.libk.o \
blocking_queue/bq_add.libk.o \
blocking_queue/bq_remove.libk.o \
map/map_create.libk.o \
map/map_delete.libk.o \
map/map_add.libk.o \
map/map_get.libk.o \
map/map_remove.libk.o \
user_lock/lock_create.libk.o \
user_lock/lock_delete.libk.o \
user_lock/lock_acquire.libk.o \
user_lock/lock_release.libk.o \
user_event/event_create.libk.o \
user_event/event_delete.libk.o \
user_event/event_wait.libk.o \
user_event/event_signal.libk.o \
user_event/event_broadcast.libk.o \
user_thread/thread_create.libk.o \
user_thread/thread_exit.libk.o \
user_thread/thread_yield.libk.o \
user_thread/thread_join.libk.o \
user_process/proc_fork.libk.o \
user_process/proc_exec.libk.o \
user_process/proc_exit.libk.o \
user_process/proc_wait.libk.o \
user_file/open.libk.o \
user_file/close.libk.o \
user_file/read.libk.o \
user_file/write.libk.o \
user_file/pipe.libk.o \
user_file/dup.libk.o \
str/str_create.libk.o \
str/str_delete.libk.o \
str/str_add_char.libk.o \
str/str_add_cstr.libk.o \
str/str_get_cstr.libk.o \
str/str_grow.libk.o \
vect/vect_create.libk.o \
vect/vect_delete.libk.o \
vect/vect_get.libk.o \
vect/vect_add.libk.o \
vect/vect_pop.libk.o \
vect/vect_grow.libk.o \
vect/vect_get_array.libk.o
#ubsan/ubsan.libk.o

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