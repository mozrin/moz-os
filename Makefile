CC=clang
CFLAGS=-O3 -ffreestanding -fno-builtin -fno-stack-protector -mno-red-zone -nostdlib -m64
LD=ld

all: build

build: boot/mbr.bin boot/stage2.bin kernel/kernel.img scripts/mkimg.sh
    bash scripts/mkimg.sh

boot/mbr.bin: boot/mbr.asm
    fasm $< $@

boot/stage2.bin: boot/stage2.asm
    fasm $< $@

kernel/kernel.img: kernel/kernel.c kernel/link.ld
    $(CC) $(CFLAGS) -c kernel/kernel.c -o kernel/kernel.o
    $(LD) -T kernel/link.ld -o kernel/kernel.img kernel/kernel.o

run: scripts/run-qemu.sh
    bash scripts/run-qemu.sh

clean:
    rm -f boot/mbr.bin boot/stage2.bin kernel/kernel.o kernel/kernel.img os.img
