/* Desc: Global function declarations and type definitions */

#ifndef __CORE_H
#define __CORE_H

/* This defines the data types available in the KinX Kernel */

typedef int size_t;

typedef enum { false, true } boolean;

#define NULLCHAR '\0'
#define NULL	 (void*)0
#define TRUE     1
#define FALSE    0

// Primitive typedef
typedef signed char          int8_t;
typedef unsigned char        uint8_t;
typedef short                int16_t;
typedef unsigned short       uint16_t;
typedef int                  int32_t;
typedef unsigned             uint32_t;
typedef long long            int64_t;
typedef unsigned long long   uint64_t;

extern int counter_0;
extern int counter_1;
extern int counter_2;
extern int counter_3;
extern int counter_4;
extern int counter_5;

unsigned char global_keyboard_char;

/* This defines what the stack looks like after an ISR was running */
struct regs
{
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;    
};

/* PHYS_MEMORY.C */
struct MULTIBOOT_INFO {
    uint32_t flags;         //required
    uint32_t memoryLo;      //if bit 0 in flags are set
    uint32_t memoryHi;      //if bit 0 in flags are set
    uint32_t bootDevice;        //if bit 1 in flags are set
    uint32_t commandLine;       //if bit 2 in flags are set
    uint32_t moduleCount;       //if bit 3 in flags are set
    uint32_t moduleAddress;     //if bit 3 in flags are set
    uint32_t syms[4];       //if bits 4 or 5 in flags are set
    uint32_t memMapLength;      //if bit 6 in flags is set
    uint32_t memMapAddress;     //if bit 6 in flags is set
    uint32_t drivesLength;      //if bit 7 in flags is set
    uint32_t drivesAddress;     //if bit 7 in flags is set
    uint32_t configTable;       //if bit 8 in flags is set
    uint32_t apmTable;      //if bit 9 in flags is set
    uint32_t vbeControlInfo;    //if bit 10 in flags is set
    uint32_t vbeModeInfo;       //if bit 11 in flags is set
    uint32_t vbeMode;       // all vbe_* set if bit 12 in flags are set
    uint32_t vbeInterfaceSeg;
    uint32_t vbeInterfaceOff;
    uint32_t vbeInterfaceLength;
};
extern struct MULTIBOOT_INFO *MULTIBOOT_INFO_STRUCTURE;
extern int memory_size;
extern void physical_memory_manager_install();
/* MEMORY.C */
extern void *memcpy(void *dest, const void *src, size_t count);
extern void *memset(void *dest, char val, size_t count);
unsigned char *memsetb(void *dest, char val, size_t count);
extern unsigned short *memsetw(unsigned short *dest, unsigned short val, size_t count);

/* GDT.C */
extern void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);
extern void gdt_install();
extern struct gdt_ptr gp;

/* IDT.C */
extern void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
extern void idt_install();

/* ISRS.C */
extern void isrs_install();

/* IRQ.C */
extern void irq_install_handler(int irq, void (*handler)(struct regs *r));
extern void irq_uninstall_handler(int irq);
extern void irq_install();

/* TIMER.C */

struct RTC {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t century;
};

extern struct RTC current_time;

#define CURRENT_YEAR 2013
//typedef enum {JAN,FEB,MAR,APR,MAY,JUN,JUL,AUG,SEP,OCT,NOV,DEC} MONTHS;

enum {
    cmos_address = 0x70,
    cmos_data    = 0x71
};

extern void timer_wait(int ticks);
extern void waits(int seconds);
extern void waitm(int minutes);
extern void timer_install();
extern void get_time();

/* KEYBOARD.C */
extern void keyboard_install();

/* KERNEL.C */
extern kmain();

#endif
