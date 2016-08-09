/*  Desc: Kernel C Code and Main file */
/* Check if the bit BIT in FLAGS is set. */
#include <multiboot.h>
#include <cpu.h>
#include <core.h>
#include <memory.h>
#include <shell.h>
#include <stdio.h>
#include <varg.h>
#include <video.h>

kmain(unsigned long magic, unsigned long info) //like normal main in C programs
{
	/*if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
		 {
		   puts ("Invalid magic number: 0x%x\n", (unsigned) magic);
		   return;
		 }*/

	/* Set MBI to the address of the Multiboot information structure. */
	//mbi = (multiboot_info_t *) info;
	init_video();
	ClearScreen();
	putch('\n');
	puts("Initiating video [DONE]\n");
	puts("Installing Global Descriptor Table");
		gdt_install();
			puts(" [DONE]\n");
	puts("Installing Interrupt Descriptor Table");
		idt_install();
			puts(" [DONE]\n");
	puts("Installing Interrupt Service Routines");
		isrs_install();
			puts(" [DONE]\n");
	puts("Installing Interrupt Vector Table");
		irq_install();
			puts(" [DONE]\n");
	puts("Installing Physical Memory Manager");
		physical_memory_manager_install(info);
			puts(" [DONE]\n");
	puts("Installing Timer Driver");
		timer_install();
			puts(" [DONE]\n");
	puts("Installing Keyboard Driver");
		keyboard_install();
			puts(" [DONE]\n");
	puts("Enabling Interrupts");
		enableInts();
			puts(" [DONE]\n");
	puts("Starting KinDOS\n");
		kinDOSInstall();

	while (1) { hlt(); }
};
