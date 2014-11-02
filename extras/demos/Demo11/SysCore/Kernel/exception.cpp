//****************************************************************************
//**
//**    exception.cpp
//**		system exception handlers. These are registered during system
//**		initialization and called automatically when they are encountered
//**
//****************************************************************************

#include "exception.h"
#include <hal.h>
#include <stdint.h>

extern void _cdecl kernel_panic (const char* fmt, ...);

//! just fixes up the stack so we can properly access cpu paramaters
#ifdef _MSC_VER
#define intstart() \
	_asm	cli \
	_asm	sub		ebp, 4
#else
#define intstart()
#endif

// warning C4100: unreferenced formal parameter
#pragma warning (disable:4100)

//warning C4731: frame pointer register 'ebp' modified by inline assembly code
#pragma warning (disable:4731)

//! divide by 0 fault
void _cdecl divide_by_zero_fault (uint32_t eip, uint32_t cs, uint32_t flags) {

	intstart ();
	kernel_panic ("Divide by 0 at physical address [0x%x:0x%x] EFLAGS [0x%x]",cs,eip, flags);
	for (;;);
}

//! single step
void _cdecl single_step_trap (uint32_t cs, uint32_t eip, uint32_t flags) {

	intstart ();
	kernel_panic ("Single step at physical address [0x%x:0x%x] EFLAGS [0x%x]",cs,eip, flags);
	for (;;);
}

//! non maskable interrupt trap
void _cdecl nmi_trap (uint32_t cs, uint32_t eip, uint32_t flags) {

	intstart ();
	kernel_panic ("NMI trap at physical address [0x%x:0x%x] EFLAGS [0x%x]",cs,eip, flags);
	for (;;);
}

//! breakpoint hit
void _cdecl breakpoint_trap (uint32_t cs, uint32_t eip, uint32_t flags) {

	intstart ();
	kernel_panic ("Breakpoint trap at physical address [0x%x:0x%x] EFLAGS [0x%x]",cs,eip, flags);
	for (;;);
}

//! overflow
void _cdecl overflow_trap (uint32_t cs, uint32_t eip, uint32_t flags) {

	intstart ();
	kernel_panic ("Overflow trap at physical address [0x%x:0x%x] EFLAGS [0x%x]",cs,eip, flags);
	for (;;);
}

//! bounds check
void _cdecl bounds_check_fault (uint32_t cs, uint32_t  eip, uint32_t flags) {

	intstart ();
	kernel_panic ("Bounds check fault at physical address [0x%x:0x%x] EFLAGS [0x%x]",cs,eip, flags);
	for (;;);
}

//! invalid opcode / instruction
void _cdecl invalid_opcode_fault (uint32_t cs, uint32_t  eip, uint32_t flags) {

	intstart ();
	kernel_panic ("Invalid opcode at physical address [0x%x:0x%x] EFLAGS [0x%x]",cs,eip, flags);
	for (;;);
}

//! device not available
void _cdecl no_device_fault (uint32_t cs, uint32_t eip, uint32_t flags) {

	intstart ();
	kernel_panic ("Device not found fault at physical address [0x%x:0x%x] EFLAGS [0x%x]",cs,eip, flags);
	for (;;);
}

//! double fault
void _cdecl double_fault_abort (uint32_t eip,uint32_t cs,uint32_t flags,uint32_t err)  {

	intstart ();
	kernel_panic ("Double fault at physical address [0x%x:0x%x] EFLAGS [0x%x]",cs,eip, flags);
	for (;;);
}

//! invalid Task State Segment (TSS)
void _cdecl invalid_tss_fault (uint32_t eip,uint32_t cs,uint32_t flags,uint32_t err) {

	intstart ();
	kernel_panic ("Invalid TSS at physical address [0x%x:0x%x] EFLAGS [0x%x]",cs,eip, flags);
	for (;;);
}

//! segment not present
void _cdecl no_segment_fault (uint32_t eip,uint32_t cs,uint32_t flags,uint32_t err)  {

	intstart ();
	kernel_panic ("Invalid segment at physical address [0x%x:0x%x] EFLAGS [0x%x]",cs,eip, flags);
	for (;;);
}

//! stack fault
void _cdecl stack_fault (uint32_t eip,uint32_t cs,uint32_t flags,uint32_t err)  {

	intstart ();
	kernel_panic ("Stack fault at physical address [0x%x:0x%x] EFLAGS [0x%x]",cs,eip, flags);
	for (;;);
}

//! general protection fault
void _cdecl general_protection_fault (uint32_t eip,uint32_t cs,uint32_t flags,uint32_t err)  {

	intstart ();
	kernel_panic ("General Protection Fault at physical address [0x%x:0x%x] EFLAGS [0x%x]",
		cs,cs, cs);
	for (;;);
}


//! page fault
void _cdecl page_fault (uint32_t eip,uint32_t cs,uint32_t flags,uint32_t err) {

	intstart ();

	int faultAddr=0;

	_asm {
		mov eax, cr2
		mov [faultAddr], eax
	}

	kernel_panic ("Page Fault at 0x%x:0x%x refrenced memory at 0x%x",
		cs, eip, faultAddr);
	for (;;);
}

//! Floating Point Unit (FPU) error
void interrupt _cdecl fpu_fault  (uint32_t cs, uint32_t  eip, uint32_t flags)  {

	intstart ();
	kernel_panic ("FPU Fault at physical address [0x%x:0x%x] EFLAGS [0x%x]",cs,eip, flags);
	for (;;);
}

//! alignment check
void interrupt _cdecl alignment_check_fault (uint32_t eip,uint32_t cs,uint32_t flags,uint32_t err)  {

	intstart ();
	kernel_panic ("Alignment Check at physical address [0x%x:0x%x] EFLAGS [0x%x]",cs,eip, flags);
	for (;;);
}

//! machine check
void interrupt _cdecl machine_check_abort  (uint32_t cs, uint32_t  eip, uint32_t flags)  {

	intstart ();
	kernel_panic ("Machine Check at physical address [0x%x:0x%x] EFLAGS [0x%x]",cs,eip, flags);
	for (;;);
}

//! Floating Point Unit (FPU) Single Instruction Multiple Data (SIMD) error
void interrupt _cdecl simd_fpu_fault (uint32_t cs, uint32_t  eip, uint32_t flags)  {

	intstart ();
	kernel_panic ("FPU SIMD fault at physical address [0x%x:0x%x] EFLAGS [0x%x]",cs,eip, flags);
	for (;;);
}
