//****************************************************************************
//**
//**	main.cpp
//**		entry point for kernel
//**
//****************************************************************************

#include <bootinfo.h>
#include <hal.h>
#include "DebugDisplay.h"
#include "exception.h"
#include "mmngr_phys.h"

//! format of a memory region
struct memory_region {

	uint32_t	startLo;
	uint32_t	startHi;
	uint32_t	sizeLo;
	uint32_t	sizeHi;
	uint32_t	type;
	uint32_t	acpi_3_0;
};

//! different memory regions (in memory_region.type)
char* strMemoryTypes[] = {

	{"Available"},			//memory_region.type==0
	{"Reserved"},			//memory_region.type==1
	{"ACPI Reclaim"},		//memory_region.type==2
	{"ACPI NVS Memory"}		//memory_region.type==3
};

int _cdecl kmain (multiboot_info* bootinfo) {

	//! get kernel size passed from boot loader
	uint32_t kernelSize=0;
	_asm mov	word ptr [kernelSize], dx

	//! make demo look nice :)
	DebugClrScr (0x13);
	DebugGotoXY (0,0);

	DebugSetColor (0x3F);
	DebugPrintf ("                    ~[ Physical Memory Manager Demo ]~                          ");

	DebugGotoXY (0,24);
	DebugSetColor (0x3F);
	DebugPrintf ("                                                                                ");

	DebugGotoXY (0,2);
	DebugSetColor (0x17);

	//! initialize hal
	hal_initialize ();

	//! enable interrupts and install exception handlers
	enable ();
	setvect (0,(void (__cdecl &)(void))divide_by_zero_fault);
	setvect (1,(void (__cdecl &)(void))single_step_trap);
	setvect (2,(void (__cdecl &)(void))nmi_trap);
	setvect (3,(void (__cdecl &)(void))breakpoint_trap);
	setvect (4,(void (__cdecl &)(void))overflow_trap);
	setvect (5,(void (__cdecl &)(void))bounds_check_fault);
	setvect (6,(void (__cdecl &)(void))invalid_opcode_fault);
	setvect (7,(void (__cdecl &)(void))no_device_fault);
	setvect (8,(void (__cdecl &)(void))double_fault_abort);
	setvect (10,(void (__cdecl &)(void))invalid_tss_fault);
	setvect (11,(void (__cdecl &)(void))no_segment_fault);
	setvect (12,(void (__cdecl &)(void))stack_fault);
	setvect (13,(void (__cdecl &)(void))general_protection_fault);
	setvect (14,(void (__cdecl &)(void))page_fault);
	setvect (16,(void (__cdecl &)(void))fpu_fault);
	setvect (17,(void (__cdecl &)(void))alignment_check_fault);
	setvect (18,(void (__cdecl &)(void))machine_check_abort);
	setvect (19,(void (__cdecl &)(void))simd_fpu_fault);

	//! get memory size in KB
	uint32_t memSize = 1024 + bootinfo->m_memoryLo + bootinfo->m_memoryHi*64;

	//! initialize the physical memory manager
	//! we place the memory bit map used by the PMM at the end of the kernel in memory
	pmmngr_init (memSize, 0x100000 + kernelSize*512);

	DebugPrintf("pmm initialized with %i KB physical memory; memLo: %i memHi: %i\n\n",
		memSize,bootinfo->m_memoryLo,bootinfo->m_memoryHi);

	DebugSetColor (0x19);
	DebugPrintf ("Physical Memory Map:\n");

	memory_region*	region = (memory_region*)0x1000;

	for (int i=0; i<15; ++i) {

		//! sanity check; if type is > 4 mark it reserved
		if (region[i].type>4)
			region[i].type=1;

		//! if start address is 0, there is no more entries, break out
		if (i>0 && region[i].startLo==0)
			break;

		//! display entry
		DebugPrintf ("region %i: start: 0x%x%x length (bytes): 0x%x%x type: %i (%s)\n", i, 
			region[i].startHi, region[i].startLo,
			region[i].sizeHi,region[i].sizeLo,
			region[i].type, strMemoryTypes[region[i].type-1]);

		//! if region is avilable memory, initialize the region for use
		if (region[i].type==1)
			pmmngr_init_region (region[i].startLo, region[i].sizeLo);
	}

	//! deinit the region the kernel is in as its in use
	pmmngr_deinit_region (0x100000, kernelSize*512);

	DebugSetColor (0x17);

	DebugPrintf ("\npmm regions initialized: %i allocation blocks; used or reserved blocks: %i\nfree blocks: %i\n",
		pmmngr_get_block_count (),  pmmngr_get_use_block_count (), pmmngr_get_free_block_count () );

	//! allocating and deallocating memory examples...

	DebugSetColor (0x12);

	uint32_t* p = (uint32_t*)pmmngr_alloc_block ();
	DebugPrintf ("\np allocated at 0x%x", p);

	uint32_t* p2 = (uint32_t*)pmmngr_alloc_blocks (2);
	DebugPrintf ("\nallocated 2 blocks for p2 at 0x%x", p2);

	pmmngr_free_block (p);
	p = (uint32_t*)pmmngr_alloc_block ();
	DebugPrintf ("\nUnallocated p to free block 1. p is reallocated to 0x%x", p);

	pmmngr_free_block (p);
	pmmngr_free_blocks (p2, 2);
	return 0;
}



