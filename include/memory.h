/* Desc: memory functions */

#ifndef __MEMORY_H
#define __MEMORY_H

/* MEMORY.C */
extern void *memcpy(void *dest, const void *src, size_t count);
extern void *memset(void *dest, char val, size_t count);
unsigned char *memsetb(void *dest, char val, size_t count);
extern unsigned short *memsetw(unsigned short *dest, unsigned short val, size_t count);

/* PHYS_MEMORY.C */
extern multiboot_info_t *mbi;
extern void physical_memory_manager_install();

#endif
