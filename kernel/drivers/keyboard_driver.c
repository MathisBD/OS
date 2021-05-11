#include <stddef.h>
#include "drivers/keyboard_driver.h"
#include "tables/idt.h"
#include "drivers/pic_driver.h"
#include "drivers/port_io.h"
#include <stdio.h>
#include <string.h>
#include "drivers/dev.h"
#include "threads/thread.h"
#include "memory/kheap.h"
#include "filesystem/file_descr.h"


// interrupt number for a keyboard interrupt
#define KEYBOARD_IRQ 1

#define KEYBOARD_COMM 0x64 // command/status port
#define KEYBOARD_DATA 0x60 // data port


// qwerty scan code set : https://wiki.osdev.org/PS/2_Keyboard#Scan_Code_Set_1
// modifying to azerty is straightforward
#define KEYCODE_CTRL 0x1D
#define KEYCODE_LSHIFT 0x2A
#define KEYCODE_RSHIFT 0x36
#define KEYCODE_ALT 0x38

static char kbd_map[128];
static char kbd_shift_map[128];
static char kbd_alt_map[128];

// lock that protects access to the keyboard function kbd_read()
static queuelock_t* kbd_lock;

// the current state of the keyboard
static uint8_t kbd_flags;
// the last key pressed
static key_t last_key;
// set to true each time a key is pressed,
// except for ctrl/shift/alt
static bool key_received;


static void init_kbd_maps() 
{
    for (int i = 0; i < 128; i++) {
        kbd_map[i] = 0;
        kbd_shift_map[i] = 0;
        kbd_alt_map[i] = 0;
    }

    // DEFAULT MAP
    // first row (top) 
    kbd_map[1] = 27; // ascii for escape
    kbd_map[2] = '&';
    kbd_map[4] = '"';
    kbd_map[5] = '\'';
    kbd_map[6] = '(';
    kbd_map[7] = '-';
    kbd_map[9] = '_';
    kbd_map[12] = ')';
    kbd_map[13] = '=';
    kbd_map[14] = '\b'; // Backspace

    // second row
    kbd_map[16] = 'a';
    kbd_map[17] = 'z';
    kbd_map[18] = 'e';
    kbd_map[19] = 'r';
    kbd_map[20] = 't';
    kbd_map[21] = 'y';
    kbd_map[22] = 'u';
    kbd_map[23] = 'i';
    kbd_map[24] = 'o';
    kbd_map[25] = 'p';
    kbd_map[26] = '^';
    kbd_map[27] = '$';
    kbd_map[28] = '\n'; // Enter

    // third row
    kbd_map[30] = 'q';
    kbd_map[31] = 's';
    kbd_map[32] = 'd';
    kbd_map[33] = 'f';
    kbd_map[34] = 'g';
    kbd_map[35] = 'h';
    kbd_map[36] = 'j';
    kbd_map[37] = 'k';
    kbd_map[38] = 'l';
    kbd_map[39] = 'm';
    kbd_map[43] = '*';

    // fourth row
    kbd_map[86] = '<';
    kbd_map[44] = 'w';
    kbd_map[45] = 'x';
    kbd_map[46] = 'c';
    kbd_map[47] = 'v';
    kbd_map[48] = 'b';
    kbd_map[49] = 'n';
    kbd_map[50] = ',';
    kbd_map[51] = ';';
    kbd_map[52] = ':';
    kbd_map[53] = '!';

    // SHIFT MAP
    // first row
    kbd_shift_map[2] = '1';
    kbd_shift_map[3] = '2';
    kbd_shift_map[4] = '3';
    kbd_shift_map[5] = '4';
    kbd_shift_map[6] = '5';
    kbd_shift_map[7] = '6';
    kbd_shift_map[8] = '7';
    kbd_shift_map[9] = '8';
    kbd_shift_map[10] = '9';
    kbd_shift_map[11] = '0';
    kbd_shift_map[13] = '+';

    // second row
    kbd_shift_map[16] = 'A';
    kbd_shift_map[17] = 'Z';
    kbd_shift_map[18] = 'E';
    kbd_shift_map[19] = 'R';
    kbd_shift_map[20] = 'T';
    kbd_shift_map[21] = 'Y';
    kbd_shift_map[22] = 'U';
    kbd_shift_map[23] = 'I';
    kbd_shift_map[24] = 'O';
    kbd_shift_map[25] = 'P';

    // third row 
    kbd_shift_map[30] = 'Q';
    kbd_shift_map[31] = 'S';
    kbd_shift_map[32] = 'D';
    kbd_shift_map[33] = 'F';
    kbd_shift_map[34] = 'G';
    kbd_shift_map[35] = 'H';
    kbd_shift_map[36] = 'J';
    kbd_shift_map[37] = 'K';
    kbd_shift_map[38] = 'L';
    kbd_shift_map[39] = 'M';
    kbd_shift_map[40] = '%';

    // fourth row
    kbd_shift_map[86] = '>';
    kbd_shift_map[44] = 'W';
    kbd_shift_map[45] = 'X';
    kbd_shift_map[46] = 'C';
    kbd_shift_map[47] = 'V';
    kbd_shift_map[48] = 'B';
    kbd_shift_map[49] = 'N';
    kbd_shift_map[50] = '?';
    kbd_shift_map[51] = '.';
    kbd_shift_map[52] = '/';

    // ALT MAP
    // first row
    kbd_alt_map[3] = '~';
    kbd_alt_map[4] = '#';
    kbd_alt_map[5] = '{';
    kbd_alt_map[6] = '[';
    kbd_alt_map[7] = '|';
    kbd_alt_map[8] = '`';
    kbd_alt_map[9] = '\\';
    kbd_alt_map[10] = '^';
    kbd_alt_map[11] = '@';
    kbd_alt_map[12] = ']';
    kbd_alt_map[13] = '}';
}


static int kbd_read(void* buf, int count)
{
    kql_acquire(kbd_lock);

    disable_irq(KEYBOARD_IRQ);
    key_received = false;
    while (!key_received) {
        enable_irq(KEYBOARD_IRQ);
        // we can't use an event here : 
        // it would be the keyboard interrupt that would wake us up,
        // and we can't use signal() or broadcast() in an interrupt
        // (because they need to acquire the heap spinlock).
        kthread_yield();
        disable_irq(KEYBOARD_IRQ);
    }
    memcpy(buf, &(last_key.c), 1);
    enable_irq(KEYBOARD_IRQ);

    kql_release(kbd_lock);
    return 1;
}


void init_kbd_driver()
{
    kbd_lock = kql_create();
    init_kbd_maps();

    kbd_flags = 0;
    key_received = false; 

    stream_dev_t* dev = kmalloc(sizeof(stream_dev_t));
    dev->lock = kql_create();
    dev->name = "kbd";
    dev->perms = FD_PERM_READ;
    dev->read = kbd_read;
    register_stream_dev(dev);
}

static void key_released(uint8_t keycode)
{
    switch (keycode) {
    case KEYCODE_ALT: kbd_flags &= ~KBD_FLAG_ALT; break;
    case KEYCODE_CTRL: kbd_flags &= ~KBD_FLAG_CTRL; break;
    case KEYCODE_LSHIFT : kbd_flags &= ~KBD_FLAG_LSHIFT; break;
    case KEYCODE_RSHIFT: kbd_flags &= ~KBD_FLAG_RSHIFT; break;
    }
}

static void key_pressed(uint8_t keycode)
{
    switch (keycode) {
    case KEYCODE_ALT: kbd_flags |= KBD_FLAG_ALT; break;
    case KEYCODE_CTRL: kbd_flags |= KBD_FLAG_CTRL; break;
    case KEYCODE_LSHIFT : kbd_flags |= KBD_FLAG_LSHIFT; break;
    case KEYCODE_RSHIFT: kbd_flags |= KBD_FLAG_RSHIFT; break;
    default:
    {
        // shift has a higher priority than alt
        // (because why not ?) 
        if ((kbd_flags & KBD_FLAG_LSHIFT) || (kbd_flags & KBD_FLAG_RSHIFT)) {
            last_key.c = kbd_shift_map[keycode];
        }
        else if (kbd_flags & KBD_FLAG_ALT) {
            last_key.c = kbd_alt_map[keycode];
        }
        else {
            last_key.c = kbd_map[keycode];
        }
        key_received = true;
        //printf("%c", last_key.c);
        break;
    }
    }
}

void kbd_interrupt()
{
	uint8_t status = port_int8_in(KEYBOARD_COMM);
    
    // bit 1 of status tells us if the output buffer is empty/full
	if (status & 0x01) {
		uint8_t keycode = port_int8_in(KEYBOARD_DATA);
        
        // key released
        if (keycode & 0x80) {
            keycode &= ~0x80;
            key_released(keycode);
        }
        // key pressed
        else {
            key_pressed(keycode);
        }
    }
}