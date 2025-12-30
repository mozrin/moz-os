# =============================================================================
# moz-os phase 0001 makefile
# =============================================================================

FASM = fasm

.PHONY: all mbr clean

all: mbr

mbr: boot/mbr.bin

boot/mbr.bin: boot/mbr.asm
	$(FASM) boot/mbr.asm boot/mbr.bin

clean:
	rm -f boot/mbr.bin
