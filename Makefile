CC=gcc
CFLAGS=-c \
	-O \
	-fstrength-reduce \
	-fomit-frame-pointer \
	-nostartfiles \
	-nodefaultlibs \
	-nostdlib \
	-finline-functions \
	-nostdinc \
	-fno-builtin \
	-std=c99 \
	-I./include \
	-m32

LDFLAGS=-T linker.ld -melf_i386

SOURCES=$(SOURCES) kernel.c
SOURCES:=$(wildcard hal/*.c)
SOURCES:=$(SOURCES) $(wildcard util/*.c)
SOURCES:=$(SOURCES) $(wildcard shell/*.c)
SOURCES:=$(SOURCES) $(wildcard main/*.c)
OBJECTS=$(SOURCES:.c=.o)
LOADER=boot/loader.o
KERNEL=KinXOS.elf
IMAGE=KinXOS.flat.img

all: $(SOURCES) $(IMAGE)

color: CC=colorgcc
color: $(SOURCES) $(IMAGE)

# Generate Image
$(IMAGE): $(KERNEL)
	@echo 'Generating Image!'
	cat boot/grub/stage1 boot/grub/stage2 boot/grub/pad KinXOS.elf > KinXOS.flat.img
	@echo 'Finished Operating System: $@'
	@echo ' '
	
# Compile the Loader Assembly File
$(KERNEL): $(LOADER) $(OBJECTS) 
	@echo 'Linking Kernel into: $@'
	@echo 'Invoking: LD'
	ld -T linker.ld -melf_i386 -o KinXOS.elf $(LOADER) $(OBJECTS)
	@echo 'Finished building target: $@'
	@echo ' '

# Compile Loader.asm file
$(LOADER): 
	@echo 'Compiling Kernel: $@'
	@echo 'Invoking: NASM'
	nasm -f elf32 -o $@ loader.asm
	@echo 'Finished building target: $@'
	@echo ' '

# Compile all Source files
.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

# Remove all object files from directory
clean:
	-@echo 'Removing all *.o files in directory'
	rm $(OBJECTS)
	rm KinXOS.elf
