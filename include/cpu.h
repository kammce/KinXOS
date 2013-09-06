#ifndef __CPU_H
#define __CPU_H

//! The following devices use PIC 1 to generate interrupts
#define		I86_PIC_IRQ_TIMER		0
#define		I86_PIC_IRQ_KEYBOARD	1
#define		I86_PIC_IRQ_SERIAL2		3
#define		I86_PIC_IRQ_SERIAL1		4
#define		I86_PIC_IRQ_PARALLEL2	5
#define		I86_PIC_IRQ_DISKETTE	6
#define		I86_PIC_IRQ_PARALLEL1	7

//! The following devices use PIC 2 to generate interrupts
#define		I86_PIC_IRQ_CMOSTIMER	0
#define		I86_PIC_IRQ_CGARETRACE	1
#define		I86_PIC_IRQ_AUXILIARY	4
#define		I86_PIC_IRQ_FPU			5
#define		I86_PIC_IRQ_HDC			6

extern unsigned char inportb (unsigned short _port);
extern void outportb (unsigned short _port, unsigned char _data);
extern void hlt(void);
//void invlpg(_dword_union address);
extern void disableInts(void);
extern void enableInts(void);
extern void reboot();

#endif
