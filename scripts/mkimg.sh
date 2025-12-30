# scripts/mkimg.sh
set -e
IMG=os.img

# 16MB zeroed image
dd if=/dev/zero of=$IMG bs=1M count=16

# Write MBR
dd if=boot/mbr.bin of=$IMG bs=512 count=1 conv=notrunc

# Stage2 at LBA 2 (sector #2)
dd if=boot/stage2.bin of=$IMG bs=512 seek=2 conv=notrunc

# Kernel starting at LBA 8 (align on 4KB)
dd if=kernel/kernel.img of=$IMG bs=512 seek=8 conv=notrunc
echo "Image built: $IMG"
