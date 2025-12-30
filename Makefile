# =============================================================================
# moz-os phase 0001 makefile
# =============================================================================

FASM = fasm

.PHONY: all mbr stage2 clean image run

all: image

mbr: boot/mbr.bin

stage2: boot/stage2.bin

boot/mbr.bin: boot/mbr.asm
	$(FASM) boot/mbr.asm boot/mbr.bin

boot/stage2.bin: boot/stage2.asm
	$(FASM) boot/stage2.asm boot/stage2.bin

image: boot/mbr.bin boot/stage2.bin
	dd if=/dev/zero of=disk.img bs=1024 count=1440
	dd if=boot/mbr.bin of=disk.img conv=notrunc
	dd if=boot/stage2.bin of=disk.img bs=512 seek=2 conv=notrunc

run: image
	qemu-system-x86_64 -drive format=raw,file=disk.img

clean:
	rm -f boot/mbr.bin boot/stage2.bin disk.img
