# scripts/run-qemu.sh
set -e
qemu-system-x86_64 -m 256 -drive format=raw,file=os.img -serial stdio
