; boot/stage2.asm
use16
org 0x8000

; Read kernel.img starting from LBA 8 (adjust in mkimg.sh) into 0x00100000
mov bx, 0x0000
mov es, bx
mov di, 0x0000

mov si, kern_dap
mov dl, [BootDrive]
mov ah, 0x42
int 0x13
jc disk_error

; Enable A20
in   al,0x92
or   al,00000010b
out  0x92,al

; Set GDT
lgdt [gdt_desc]

; Protected mode
mov eax, cr0
or  eax, 1
mov cr0, eax
jmp 0x08:pmode

use32
pmode:
mov ax, 0x10
mov ds, ax
mov es, ax
mov ss, ax

; Enable PAE
mov eax, cr4
or  eax, 1<<5
mov cr4, eax

; Page tables (identity map first 1GiB)
mov eax, pml4
mov cr3, eax

; Enable long mode
mov ecx, 0xC0000080
rdmsr
or  eax, 1
wrmsr

; Paging on
mov eax, cr0
or  eax, 1<<31
mov cr0, eax

jmp 0x08:long_mode

use64
long_mode:
mov rax, 0x00100000
jmp rax

disk_error:
hlt
jmp $

; --- Data ---
BootDrive db 0

align 16
gdt:
  dq 0x0000000000000000
  dq 0x00AF9A000000FFFF
  dq 0x00AF92000000FFFF
gdt_desc:
  dw (gdt_end - gdt - 1)
  dd gdt
gdt_end:

align 4096
pml4: dq pdpt | 0x003, 0,0,0,0,0,0,0
align 4096
pdpt: dq pd | 0x003, 0,0,0,0,0,0,0
align 4096
pd:   dq (0x00000000) | (1<<7) | 0x003, 0,0,0,0,0,0,0

align 16
kern_dap:
db 0x10,0x00
dw 64                 ; sectors to read (adjust if kernel grows)
dw 0x0000             ; offset ignored for EDD, we pass linear addr below
dw 0x0000
dq 8                  ; LBA start of kernel
