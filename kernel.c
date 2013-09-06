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
           printf ("Invalid magic number: 0x%x\n", (unsigned) magic);
           return;
         }*/

    /* Set MBI to the address of the Multiboot information structure. */
    //mbi = (multiboot_info_t *) info;
    gdt_install();
    idt_install();
    isrs_install();
    irq_install();
    physical_memory_manager_install(info);
    init_video();
    timer_install();
    keyboard_install();
	enableInts();
	kin_dos_install();

	while (1) { hlt(); }
};
