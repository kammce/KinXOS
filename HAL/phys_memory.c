/*  Desc: */

#include <core.h>

struct MULTIBOOT_INFO *MULTIBOOT_INFO_STRUCTURE;
int memory_size = 1024; 
/* This is a global that exists in the loader.asm file */
void physical_memory_manager_install(struct MULTIBOOT_INFO *info) {
	MULTIBOOT_INFO_STRUCTURE = info;
	memory_size = 1024 + MULTIBOOT_INFO_STRUCTURE->memoryLo + MULTIBOOT_INFO_STRUCTURE->memoryHi*64;
}