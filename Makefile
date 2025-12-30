# =============================================================================
# moz-os phase 0001 makefile
# =============================================================================

FASM = fasm

.PHONY: all mbr stage2 clean image run

all: image

# Assemble MBR
mbr:
	fasm boot/mbr.asm boot/mbr.bin

# Assemble Stage-2
stage2:
	fasm boot/stage2.asm boot/stage2.bin

# Compile Kernel
kernel_bin:
	gcc -c kernel/kernel.c -o kernel/kernel.o -ffreestanding -mno-red-zone
	ld -n -T kernel/linker.ld -o kernel/kernel.bin kernel/kernel.o --oformat binary

# Create Disk Image
# MBR at 0
# Stage2 at offset 1024 (LBA 2)
# Kernel at offset 4096 (LBA 8)
image: mbr stage2 kernel_bin
	dd if=/dev/zero of=disk.img bs=1024 count=1440
	dd if=boot/mbr.bin of=disk.img conv=notrunc
	dd if=boot/stage2.bin of=disk.img bs=512 seek=2 conv=notrunc
	dd if=kernel/kernel.bin of=disk.img bs=512 seek=8 conv=notrunc

# Run in QEMU
run: image
	qemu-system-x86_64 -drive format=raw,file=disk.img

clean:
	rm -f boot/mbr.bin boot/stage2.bin kernel/kernel.o kernel/kernel.bin disk.img
