/*
 * MEMORY.C
 * Desc: memory functions
 */
#ifndef __MEMORY_H
#define __MEMORY_H

#include <core.h>

/* because C cannot overload */
//void memcpy(void * dest, void * src, int start, int len);
void memcpy(void * dest, void * src, int len);
void memclr(void * addr, int length);
void memset(void * dest, uint8_t value, int len);
void memsetb(void *dest, char val, size_t count);
void memsetw(unsigned short *dest, unsigned short val, size_t count);

#endif
