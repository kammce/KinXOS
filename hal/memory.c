#include <core.h>
#include <memory.h>
/*
void memcpy(void * dest, void * src, int start, int len) {
    char *dest_t = (char *)dest;
    char *src_t = (char *)(src+start);
    for (;len != 0; --len) {
        *dest_t++ = *src_t++;
    }
}*/
void memcpy(void * dest, void * src, int len) {
    char *dest_t = (char *)dest;
    char *src_t = (char *)src;
    for (;len != 0; --len) { *dest_t++ = *src_t++; }
}

void memclr(void * addr, int len) {
    char *temp = (char *)addr;
    for (;len != 0; --len) { *temp++ = 0; }
}

void memset(void * dest, uint8_t value, int len) {
    char *dest_t = (char *)dest;
    for (;len != 0; --len) { *dest_t++ = value; }
}

void memsetb(void *dest, char val, size_t count)
{
    unsigned char *temp = (unsigned char *)dest;
    for( ;count != 0; count--) { *temp++ = val; }
}

void memsetw(unsigned short *dest, unsigned short val, size_t count)
{
    unsigned short *temp = (unsigned short *)dest;
    for( ; count != 0; count--) { *temp++ = val; }
}