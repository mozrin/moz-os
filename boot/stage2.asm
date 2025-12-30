; =============================================================================
; moz-os phase 0002: stage-2 loader placeholder
; =============================================================================

format binary
use16
org 0x8000

start:
    ; -------------------------------------------------------------------------
    ; 1. initialize segments
    ; -------------------------------------------------------------------------
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x8000      ; stack grows down from 0x8000 (below stage 2)
    sti

    ; -------------------------------------------------------------------------
    ; 2. enable a20 (fast a20 - port 0x92)
    ; -------------------------------------------------------------------------
    in al, 0x92
    or al, 2
    out 0x92, al

    ; -------------------------------------------------------------------------
    ; 3. load gdt
    ; -------------------------------------------------------------------------
    lgdt [gdt_descriptor]

    ; -------------------------------------------------------------------------
    ; 4. print banner (a20)
    ; -------------------------------------------------------------------------
    mov si, banner_msg
    call print_string

    ; -------------------------------------------------------------------------
    ; 5. print banner (gdt)
    ; -------------------------------------------------------------------------
    mov si, banner_gdt
    call print_string

    ; -------------------------------------------------------------------------
    ; 6. halt
    ; -------------------------------------------------------------------------
halt_loop:
    hlt
    jmp halt_loop

; =============================================================================
; routine: print_string
; input: si = pointer to null-terminated string
; =============================================================================
print_string:
    push ax
    push si
.next_char:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0e
    int 0x10
    jmp .next_char
.done:
    pop si
    pop ax
    ret

; =============================================================================
; data
; =============================================================================
banner_msg db 'stage2: a20 line enabled', 13, 10, 0
banner_gdt db 'stage2: gdt loaded', 13, 10, 0

; =============================================================================
; gdt
; =============================================================================
align 4
gdt_start:
    ; null descriptor
    dq 0x0000000000000000

    ; code descriptor: base=0, limit=0xfffff, access=0x9a, flags=0xc
    ; limit(0-15)=0xffff, base(0-15)=0x0000
    ; base(16-23)=0x00, access=0x9a, limit(16-19)=0xf, flags=0xc(0xc0 implied?), base(24-31)=0x00
    ; dw 0xffff, 0x0000, 0x9a00, 0x00cf ; flat mode 4gb
    ; prompt says flags=0x9a for access. limit=0xfffff. 
    ; let's adhere to typical 32-bit protected mode flat descriptors.
    ; access 0x9a: pr=1, priv=00, s=1, ex=1, dc=0, rw=1, ac=0
    ; flags 0xc: gr=1 (4k), sz=1 (32-bit). 
    ; so low dword: 0x0000ffff (base low, limit low)
    ; high dword: 0x00cf9a00 (base high, access, limit high/flags, base mid) -> wait, little endian
    ; db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00

    ; data descriptor: base=0, limit=0xfffff, access=0x92, flags=0xc
    ; access 0x92: pr=1, priv=00, s=1, ex=0, dc=0, rw=1, ac=0
    ; db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; limit (size - 1)
    dd gdt_start                ; base address
