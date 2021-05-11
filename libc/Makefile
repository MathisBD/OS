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

LIBC_CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -O0
LIBC_CPPFLAGS:=$(CPPFLAGS) -D__is_libc \
-I$(SYSROOT)$(INCLUDEDIR)/libc \
-Iinclude_internal

OBJS= \
stdio/printf.o \
stdio/putchar.o \
stdio/fputchar.o \
stdio/fprintf.o \
stdio/print_mem.o \
string/memcmp.o \
string/memcpy.o \
string/memset.o \
string/memmove.o \
string/strlen.o \
panic/panic.o \
stdlib/div.o \
stdlib/itoa.o \
bitset/bitset_create.o \
bitset/bitset_set.o \
bitset/bitset_clear.o \
bitset/bitset_find_one.o \
bitset/bitset_find_zero.o \
bitset/bitset_test.o \
list/list_create.o \
list/list_delete.o \
list/list_empty.o \
list/list_add_front.o \
list/list_add_back.o \
list/list_pop_front.o \
list/list_pop_back.o \
list/list_remove_node.o \
blocking_queue/bq_create.o \
blocking_queue/bq_delete.o \
blocking_queue/bq_add.o \
blocking_queue/bq_remove.o \
map/map_create.o \
map/map_delete.o \
map/map_add.o \
map/map_get.o \
map/map_remove.o \
user_lock/lock_create.o \
user_lock/lock_delete.o \
user_lock/lock_acquire.o \
user_lock/lock_release.o \
user_event/event_create.o \
user_event/event_delete.o \
user_event/event_wait.o \
user_event/event_signal.o \
user_event/event_broadcast.o \
user_thread/thread_create.o \
user_thread/thread_exit.o \
user_thread/thread_yield.o \
user_thread/thread_join.o \
user_process/proc_fork.o \
user_process/proc_exec.o \
user_process/proc_exit.o \
user_process/proc_wait.o \
user_file/open.o \
user_file/close.o \
user_file/read.o \
user_file/write.o \
user_file/pipe.o \
user_file/dup.o \
str/str_create.o \
str/str_delete.o \
str/str_add_char.o \
str/str_add_cstr.o \
str/str_get_cstr.o \
str/str_grow.o \
vect/vect_create.o \
vect/vect_delete.o \
vect/vect_get.o \
vect/vect_add.o \
vect/vect_pop.o \
vect/vect_grow.o
#ubsan/ubsan.o


BINARIES=libc.a libk.a

.PHONY: all clean install install-headers install-libs

all: $(BINARIES)

# LIBC
libc.a: $(OBJS:.o=.libc.o) 
	@$(AR) rcs $@ $(OBJS:.o=.libc.o) 

%.libc.o: %.c 
	@$(CC) -MD -c $< -o $@ -std=gnu99 $(LIBC_CFLAGS) $(LIBC_CPPFLAGS)

%.libc.o: %.S 
	@$(CC) -MD -c $< -o $@ -std=gnu99 $(LIBC_CFLAGS) $(LIBC_CPPFLAGS)

# LIBK
libk.a: $(OBJS:.o=.libk.o)
	@$(AR) rcs $@ $(OBJS:.o=.libk.o)

%.libk.o: %.c 
	@$(CC) -MD -c $< -o $@ -std=gnu99 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

%.libk.o: %.S 
	@$(CC) -MD -c $< -o $@ -std=gnu99 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)


install: install-headers install-libs 

install-headers:
	@mkdir -p $(SYSROOT)$(INCLUDEDIR)/libc 
	@cp -R --preserve=timestamps include/. $(SYSROOT)$(INCLUDEDIR)/libc/.

install-libs: $(BINARIES)
	@mkdir -p $(SYSROOT)$(LIBDIR)
	@cp $(BINARIES) $(SYSROOT)$(LIBDIR)

clean:
	@rm -f $(BINARIES)
	@rm -f $(OBJS:.o=.libk.o) $(OBJS:.o=.libc.o)
	@rm -f $(OBJS:.o=.libk.d) 

-include $(OBJS:.o=.libk.d)
-include $(OBJS:.o=.libc.d)

