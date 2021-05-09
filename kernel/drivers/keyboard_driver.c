#include <stddef.h>

#include "drivers/keyboard_driver.h"
#include "tables/idt.h"
#include "drivers/pic_driver.h"
#include "drivers/port_io.h"
#include <stdio.h>
#include "drivers/dev.h"


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

static kbd_state_t kbd_state;

// circular buffer of characters that the interrupt fills
// and the worker thread sends to the read buffer.
#define CACHE_CAPACITY 256
static char cache[CACHE_CAPACITY];
static uint32_t cache_start;
static uint32_t cache_size;
static tid_t worker_tid;

static blocking_queue_t* read_buf;

static void init_keyboard_maps() 
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


void init_keyboard_driver()
{
    init_keyboard_maps();

    kbd_state.alt_pressed = false;
    kbd_state.ctrl_pressed = false;
    kbd_state.lshift_pressed = false;
    kbd_state.rshift_pressed = false;

    cache_start = 0;
    cache_size = 0;
}

// this will run in a separate thread
void worker(int arg)
{
    while (true) {
        thread_yield();

        // the keyboard interrupt woke us up
        disable_kbd_interrupts();
        uint32_t count = min(cache_size, CACHE_CAPACITY - cache_start);
        enable_kbd_interrupts();

        // this call may block us for a while
        bq_add(read_buf, cache + cache_start, count);

        disable_kbd_interrupts();
        cache_start = (cache_start + count) % CACHE_CAPACITY;
        cache_size -= count;
        enable_kbd_interrupts();
    }
}

void register_keyboard()
{
    stream_dev_t* dev = register_stream_dev("kbd", DEV_FLAG_READ);
    read_buf = dev->read_buf;
    worker_tid = thread_create(worker, 0);
}


static void key_released(uint8_t keycode)
{
    switch (keycode) {
    case KEYCODE_ALT: kbd_state.alt_pressed = false; break;
    case KEYCODE_CTRL: kbd_state.ctrl_pressed = false; break;
    case KEYCODE_LSHIFT : kbd_state.lshift_pressed = false; break;
    case KEYCODE_RSHIFT: kbd_state.rshift_pressed = false; break;
    }
}

static void key_pressed(uint8_t keycode)
{
    switch (keycode) {
    case KEYCODE_CTRL: kbd_state.ctrl_pressed = true; break;
    case KEYCODE_LSHIFT: kbd_state.lshift_pressed = true; break;
    case KEYCODE_RSHIFT: kbd_state.rshift_pressed = true; break;
    case KEYCODE_ALT: kbd_state.alt_pressed = true; break;
    default:
    {
        char c;
        // shift has a higher priority than alt
        // (because why not ?) 
        if (kbd_state.lshift_pressed || kbd_state.rshift_pressed) {
            c = kbd_shift_map[keycode];
        }
        else if (kbd_state.alt_pressed) {
            c = kbd_alt_map[keycode];
        }
        else {
            c = kbd_map[keycode];
        }
        // store the character in the cache
        if (cache_size < CACHE_CAPACITY) {
            cache[(cache_start + cache_size) % CACHE_CAPACITY] = c;
            cache_size++;
        }
        else {
            panic("keyboard cache is full\n");
        }
        // wake up the worker if he was waiting.
        sched_try_wake_up(worker_tid);
        break;
    }
    }
}

void keyboard_interrupt()
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