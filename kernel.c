/*  Desc: Kernel C Code and Main file */

#include <cpu.h>
#include <core.h>
#include <shell.h>
#include <stdio.h>
#include <varg.h>
#include <video.h>

kmain(struct MULTIBOOT_INFO* info, uint32_t *magic) //like normal main in C programs
{
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
