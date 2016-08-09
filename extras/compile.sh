#! /bin/sh

echo "Assembling kernel:"
nasm -f elf32 -o loader.o loader.asm
#nasm -f elf -o 90x60.o 90x60.asm

echo "\nCompiling kernel:"

colorgcc -o kernel.o -c kernel.c -O -fstrength-reduce -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -finline-functions -nostdinc -fno-builtin -I./include -m32

colorgcc -o memory.o -c HAL/memory.c -O -fstrength-reduce -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -finline-functions -nostdinc -fno-builtin -I./include -m32

colorgcc -o phys_memory.o -c HAL/phys_memory.c -O -fstrength-reduce -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -finline-functions -nostdinc -fno-builtin -I./include -m32

colorgcc -o gdt.o -c HAL/gdt.c -O -fstrength-reduce -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -finline-functions -nostdinc -fno-builtin -I./include -m32

colorgcc -o idt.o -c HAL/idt.c -O -fstrength-reduce -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -finline-functions -nostdinc -fno-builtin -I./include -m32

colorgcc -o isrs.o -c HAL/isrs.c -O -fstrength-reduce -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -finline-functions -nostdinc -fno-builtin -I./include -m32

colorgcc -o irq.o -c HAL/irq.c -O -fstrength-reduce -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -finline-functions -nostdinc -fno-builtin -I./include -m32

colorgcc -o cpu.o -c HAL/cpu.c -O -fstrength-reduce -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -finline-functions -nostdinc -fno-builtin -I./include -m32

colorgcc -o timer.o -c HAL/timer.c -O -fstrength-reduce -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -finline-functions -nostdinc -fno-builtin -I./include -m32

colorgcc -o keyboard_driver.o -c HAL/keyboard_driver.c -O -fstrength-reduce -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -finline-functions -nostdinc -fno-builtin -I./include -m32
# util

colorgcc -o math.o -c util/math.c -O -fstrength-reduce -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -finline-functions -nostdinc -fno-builtin -I./include -m32

# shell

colorgcc -o stdio.o -c shell/stdio.c -O -fstrength-reduce -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -finline-functions -nostdinc -fno-builtin -I./include -m32

colorgcc -o console.o -c shell/console.c -O -fstrength-reduce -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -finline-functions -nostdinc -fno-builtin -I./include -m32

colorgcc -o commands.o -c shell/commands.c -O -fstrength-reduce -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -finline-functions -nostdinc -fno-builtin -I./include -m32

colorgcc -o KinDOS.o -c shell/KinDOS.c -O -fstrength-reduce -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -finline-functions -nostdinc -fno-builtin -I./include -m32

colorgcc -o echo.o -c shell/echo.c -O -fstrength-reduce -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -finline-functions -nostdinc -fno-builtin -I./include -m32

colorgcc -o date.o -c shell/date.c -O -fstrength-reduce -fomit-frame-pointer -nostartfiles -nodefaultlibs -nostdlib -finline-functions -nostdinc -fno-builtin -I./include -m32

echo "\nLinking your kernel:"

#ld -T linker.ld -o kernel.elf loader.o kernel.o cpu.o gdt.o idt.o isrs.o irq.o timer.o keyboard_driver.o console.o KinDOS.o echo.o math.o date.o -melf_i386

ld -T linker.ld -o kernel.elf loader.o kernel.o memory.o phys_memory.o cpu.o gdt.o idt.o isrs.o irq.o timer.o keyboard_driver.o math.o stdio.o console.o commands.o KinDOS.o echo.o date.o -melf_i386

echo "\nApending bootloader (grub):"
cat boot/grub/stage1 boot/grub/stage2 boot/grub/pad > grub_boot_loader
cat grub_boot_loader kernel.elf > KinX.img

echo "\nCleaning up (removing *.o files):"
rm *.o

echo "\ndone"
