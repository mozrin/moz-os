; boot/mbr.asm
use16
org 0x7C00

xor ax, ax
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7C00

mov si, msg
call puts

; Read stage2 (one sector) from LBA 2 to 0000:8000
mov bx, 0x8000
mov dl, [BootDrive]
mov si, dap
mov ah, 0x42
int 0x13
jc disk_error

jmp 0x0000:0x8000

puts:
lodsb
test al, al
jz .done
mov ah, 0x0E
int 0x10
jmp puts
.done: ret

disk_error:
mov si, err
call puts
jmp $

BootDrive db 0
msg db "MBR: loading stage2...",0
err db "Disk read error",0

align 16
dap:
db 0x10          ; size
db 0x00
dw 1             ; sectors
dw 0x8000        ; buffer offset
dw 0x0000        ; buffer segment
dq 2             ; LBA start

times 510-($-$$) db 0
dw 0xAA55
