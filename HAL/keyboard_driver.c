/* Desc: Keyboard driver */
#include <core.h>
#include <cpu.h>

/* KBDUS means US Keyboard Layout. This is a scancode table
*  used to layout a standard US keyboard. I have left some
*  comments in to give you an idea of what key is what, even
*  though I set it's array index to 0. You can change that to
*  whatever you want using a macro, if you wish! */

int ledFlag;
int det_shift;
int det_ctrl;

const char keymap[] = {
    0, 0x1b, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
    '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
    '\n', 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'',
    '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    0, 0, ' '
};


const char shiftmap[] = {
    0, 0x1b, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
    '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',
    '\n', 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' '
};
/* Handles the keyboard interrupt */
void keyboard_handler(struct regs *r)
{
    unsigned char scancode;

    /* Read from the keyboard's data buffer */
    scancode = inportb(0x60);

	if(scancode == 0x2a || scancode == 0x36) // left and right shift
	{
		det_shift = 1;
		scancode = 0;
	}
	if(scancode == 0xaa || scancode == 0xb6) // release of l/r shift
	{
		det_shift = 0;
		scancode = 0;
	}
	/*if(scancode == 0x45) // num lock pressed
	{
		if(ledFlag & NUM_LOCK)
			ledFlag = (ledFlag & ~NUM_LOCK);
		else
			ledFlag = (ledFlag | NUM_LOCK);
		scancode = 0;
		setLEDS(&ledFlag);
	}
	if(scancode == 0x46) // scroll lock pressed
	{
		if(ledFlag & SCROLL_LOCK)
			ledFlag = ledFlag & ~SCROLL_LOCK;
		else
			ledFlag = ledFlag | SCROLL_LOCK;
		scancode = 0;
		setLEDS(&ledFlag);
	}
	if(scancode == 0x3a) // caps lock pressed
	{
		if(ledFlag & CAPS_LOCK)
			ledFlag = ledFlag & ~CAPS_LOCK;
		else
			ledFlag = ledFlag | CAPS_LOCK;
		scancode = 0;
		setLEDS(&ledFlag);
	}*/
	if(scancode == 88) // f12 - dump process list
	{
		//dumpProcesses();
		scancode = 0;
	}
	if(scancode == 29) // CTRL
	{
		det_ctrl = 1;
		scancode = 0;
	}
	if(scancode == 157) // release CTRL
	{
		det_ctrl = 0;
		scancode = 0;
	}
	// We can't handle anything greater than 0x80
	if(scancode > 0x80) {
		scancode = 0;
	}

    /*else if (scancode && global_ret_keyboard == 1)
    {
		global_ret_keyboard = 0;
    }*/
        /* Here, a key was just pressed. Please note that if you
        *  hold a key down, you will get repeated key press
        *  interrupts. */

        /* Just to show you how this works, we simply translate
        *  the keyboard scancode into an ASCII value, and then
        *  display it to the screen. You can get creative and
        *  use some flags to see if a shift is pressed and use a
        *  different layout, or you can add another 128 entries
        *  to the above layout to correspond to 'shift' being
        *  held. If shift is held using the larger lookup table,
        *  you would add 128 to the scancode when you look for it */
	if(scancode != 0) {
		if(det_shift == 0) { global_keyboard_char = keymap[scancode]; }
		if(det_shift == 1) { global_keyboard_char = shiftmap[scancode]; }
		//putch(global_keyboard_char);
		buffer_stack(global_keyboard_char);
	}
}

/* Installs the keyboard handler into IRQ1 */
void keyboard_install()
{
    irq_install_handler(I86_PIC_IRQ_KEYBOARD, keyboard_handler);
}
