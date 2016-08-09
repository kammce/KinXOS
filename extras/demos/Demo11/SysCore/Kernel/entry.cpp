/******************************************************************************

	entry.cpp
		-Kernel entry point called from boot loader

******************************************************************************/

#include <bootinfo.h>

//! basic crt initilization stuff
extern void __cdecl  InitializeConstructors ();
extern void __cdecl  Exit ();

//! main
extern int _cdecl kmain (multiboot_info* bootinfo);

//! kernel entry point is called by boot loader
void __cdecl  kernel_entry (multiboot_info* bootinfo) {

#ifdef ARCH_X86

	// Set registers for protected mode
	_asm {
		cli
		mov ax, 10h
		mov ds, ax
		mov es, ax
		mov fs, ax
		mov gs, ax
	}
#endif //ARCH_X86

	InitializeConstructors();
	kmain (bootinfo);
	Exit ();

#ifdef ARCH_X86
	_asm {
		cli
		hlt
	}
#endif

	for (;;);
}
