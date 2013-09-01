#include <cpu.h>
#include <core.h>

int counter_0;
int counter_1;
int counter_2;
int counter_3;
int counter_4;
int counter_5;

unsigned char inportb (unsigned short _port)
{
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}
void outportb (unsigned short _port, unsigned char _data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}
void hlt(void)
{
	__asm__ __volatile__("hlt" : : : "memory");
}
/*void invlpg(_dword_union address)
{
	__asm__ __volatile__("invlpg (%0)" : : "r"(address.u));
}*/
void disableInts(void)
{
	__asm__ __volatile__("cli");
}
void enableInts(void)
{
	__asm__ __volatile__("sti");
}
void reboot()
{
    uint8_t good = 0x02;
    while (good & 0x02) {
        good = inportb(0x64);
    }
    outportb(0x64, 0xFE);

    hlt();
}