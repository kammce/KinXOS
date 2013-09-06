#include <multiboot.h>
#include <core.h>
#include <memory.h>
#include <shell.h>

/*unsigned char echo_buffer[100];
unsigned short int new_line = 0;*/

void echo (unsigned char *c1) {
	puts(c1);
}
void output_ram() {
	printf("Memory Size Lower = %d kb :: Memory Size Upper = %d kb\n",100, 10000);
}
void output_stats() {
	/* Am I booted by a Multiboot-compliant boot loader? */
	/*if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		printf ("Invalid magic number: 0x%x\n", (unsigned) magic);
		return;
	}*/
	/* Print out the flags. */
	printf ("flags = 0x%x\n", (unsigned) mbi->flags);

	/* Are mem_* valid? */
	//if (CHECK_FLAG (mbi->flags, 0)) {
		printf ("mem_lower = %uKB, mem_upper = %uKB\n",
						(unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);
		unsigned int memory_size = 1024+(unsigned)mbi->mem_lower + (unsigned)mbi->mem_upper*64;
		printf ("total memory = %uKB\n", memory_size);
	//}

	/* Is boot_device valid? */
	//if (CHECK_FLAG (mbi->flags, 1)) {
		printf ("boot_device = 0x%x\n", (unsigned) mbi->boot_device);
	//}

	/* Is the command line passed? */
	//if (CHECK_FLAG (mbi->flags, 2)) {
		printf ("cmdline = %s\n", (char *) mbi->cmdline);
	//}

	/* Are mods_* valid? */
	//if (CHECK_FLAG (mbi->flags, 3))
	//{
		module_t *mod;
		int i;

		printf ("mods_count = %d, mods_addr = 0x%x\n",
						(int) mbi->mods_count, (int) mbi->mods_addr);
		for (i = 0, mod = (module_t *) mbi->mods_addr; i < mbi->mods_count; i++, mod++) {
			printf (" mod_start = 0x%x, mod_end = 0x%x, string = %s\n",
					(unsigned) mod->mod_start,
					(unsigned) mod->mod_end,
					(char *) mod->string);
		}
	//}

	/* Bits 4 and 5 are mutually exclusive! */
	//if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
	//{
		printf ("Both bits 4 and 5 are set.\n");
		return;
	//}

	/* Is the symbol table of a.out valid? */
	//if (CHECK_FLAG (mbi->flags, 4))
	//{
		aout_symbol_table_t *aout_sym = &(mbi->u.aout_sym);

		printf ("aout_symbol_table: tabsize = 0x%0x, "
						"strsize = 0x%x, addr = 0x%x\n",
						(unsigned) aout_sym->tabsize,
						(unsigned) aout_sym->strsize,
						(unsigned) aout_sym->addr);
	//}

	/* Is the section header table of ELF valid? */
	//if (CHECK_FLAG (mbi->flags, 5))
	//{
		elf_section_header_table_t *elf_sec = &(mbi->u.elf_sec);

		printf ("elf_sec: num = %u, size = 0x%x,"
						" addr = 0x%x, shndx = 0x%x\n",
						(unsigned) elf_sec->num, (unsigned) elf_sec->size,
						(unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
	//}

	/* Are mmap_* valid? */
	//if (CHECK_FLAG (mbi->flags, 6))
	//{
		memory_map_t *mmap;

		printf ("mmap_addr = 0x%x, mmap_length = 0x%x\n",
						(unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
		for (mmap = (memory_map_t *) mbi->mmap_addr;
				 (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
				 mmap = (memory_map_t *) ((unsigned long) mmap
																	+ mmap->size + sizeof (mmap->size)))
			printf (" size = 0x%x, base_addr = 0x%x%x,"
							" length = 0x%x%x, type = 0x%x\n",
							(unsigned) mmap->size,
							(unsigned) mmap->base_addr_high,
							(unsigned) mmap->base_addr_low,
							(unsigned) mmap->length_high,
							(unsigned) mmap->length_low,
							(unsigned) mmap->type);
	//}
}