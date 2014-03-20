/* Desc: Global function declarations and type definitions */

#ifndef __CORE_H
#define __CORE_H

/* This defines the data types available in the KinX Kernel */

#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

typedef int size_t;

typedef enum { false, true } boolean;

#define NULLCHAR '\0'
#define NULL	 (void*)0
#define TRUE     1
#define FALSE    0
//#define sizeof(type) (char *)(&type+1)-(char*)(&type)

// Primitive typedef
typedef signed char          int8_t;
typedef unsigned char        uint8_t;
typedef short                int16_t;
typedef unsigned short       uint16_t;
typedef int                  int32_t;
typedef unsigned             uint32_t;
typedef long long            int64_t;
typedef unsigned long long   uint64_t;

int counter_0;
int counter_1;
int counter_2;
int counter_3;
int counter_4;
int counter_5;

int _ecx0;
int _ecx1;
int _ecx2;
int _ecx3;
int _ecx4;
int _ecx5;

unsigned char global_keyboard_char;

/* This defines what the stack looks like after an ISR was running */
struct regs
{
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
};

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
