#include <cpu.h>
#include <core.h>

int counter_0, counter_1, counter_2, counter_3, counter_4, counter_5;
int _ecx0, _ecx1, _ecx2, _ecx3, _ecx4, _ecx5;

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