/*  Desc: */

#include <multiboot.h>
#include <core.h>
#include <memory.h>

multiboot_info_t *mbi;
/* This is a global that exists in the loader.asm file  */
void physical_memory_manager_install(multiboot_info_t *MULTIBOOT_INFO_STRUCTURE) {
  mbi = MULTIBOOT_INFO_STRUCTURE;
  //memory_size = 1024 + mbi->mem_lower + mbi->mem_upper*64;
}