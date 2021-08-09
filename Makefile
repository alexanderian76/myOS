all:
	nasm -f elf64 boot.asm
	nasm -f elf64 kernel.asm
	nasm -f elf64 long_mode_init.asm
	gcc -ffreestanding -nostdlib -m64 -gdwarf-4 -ggdb3 -fno-stack-protector -c main.c  -o main.o
	gcc -ffreestanding -nostdlib -m64 -gdwarf-4 -ggdb3 -fno-stack-protector -c utils.c  -o utils.o
	gcc -ffreestanding -nostdlib -m64 -gdwarf-4 -ggdb3 -fno-stack-protector -c test_process.c  -o test_process.o
	ld -n -o isofiles/boot/kernel.bin -nmagic -T linker.ld kernel.o boot.o long_mode_init.o main.o utils.o test_process.o

clean:
	rm kernel.o boot.o main.o utils.o test_process.o long_mode_init.o isofiles/boot/kernel.bin
	
build:
	grub-mkrescue -o os.iso isofiles
	qemu-system-x86_64 -cdrom os.iso
