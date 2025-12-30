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
    ; 6. halt (removed, fallthrough to pm)
    ; -------------------------------------------------------------------------

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

    ; code descriptor 64-bit: base=0, limit=0, access=0x9a, flags=0xa (long mode)
    ; access: 0x9a (present, ring0, code, exec/read)
    ; flags: 0xa (long mode=1, db=0) -> 0xaf (long mode, present? no, flags upper nibble)
    ; upper byte of high dword: 0xaf
    ; actually, limit is ignored in 64-bit mode.
    ; let's use:
    ; dw 0xffff, 0x0000, 0x9a00, 0x00af
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xaf, 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; limit (size - 1)
    dd gdt_start                ; base address

; =============================================================================
; 4. protected mode entry
; =============================================================================
CODE_SEG equ gdt_start + 8
DATA_SEG equ gdt_start + 16

    cli
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp CODE_SEG:init_pm

; =============================================================================
; 32-bit protected mode
; =============================================================================
use32
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000        ; move stack to safe place

    ; -------------------------------------------------------------------------
    ; 5. print banner (pm) using vga
    ; -------------------------------------------------------------------------
    mov esi, banner_pm
    mov edi, 0xb8000 + 160 * 3 ; line 3 (approx)
    call print_string_pm

    ; -------------------------------------------------------------------------
    ; 6. setup identity paging (first 1gb)
    ; -------------------------------------------------------------------------
    ; we will use:
    ; pml4 at 0x1000
    ; pdpt at 0x2000
    ; pd   at 0x3000
    ; assume memory is usable there.

    ; clear paging area (0x1000 - 0x4000) - 3 pages (12kb) = 3072 dwords
    mov edi, 0x1000
    xor eax, eax
    mov ecx, 3072
    rep stosd

    ; setup pml4 at 0x1000
    ; entry 0 -> pdpt at 0x2000 | present (1) | rw (2) = 0x2003
    mov edi, 0x1000
    mov eax, 0x2003
    stosd

    ; setup pdpt at 0x2000
    ; entry 0 -> pd at 0x3000 | present (1) | rw (2) = 0x3003
    mov edi, 0x2000
    mov eax, 0x3003
    stosd

    ; setup pd at 0x3000
    ; map 512 entries (1gb) as 2mb huge pages
    mov edi, 0x3000
    mov eax, 0x83       ; present (1) | rw (2) | huge (0x80)
    mov ecx, 512

.fill_pd:
    stosd               ; store low 32 bits (address + flags)
    push eax
    mov eax, 0          ; high 32 bits (0 for now, < 4gb)
    stosd
    pop eax
    add eax, 0x200000   ; increment by 2mb
    loop .fill_pd

    ; -------------------------------------------------------------------------
    ; 7. load cr3
    ; -------------------------------------------------------------------------
    mov eax, 0x1000
    mov cr3, eax

    ; -------------------------------------------------------------------------
    ; 8. print banner (paging)
    ; -------------------------------------------------------------------------
    mov esi, banner_paging
    mov edi, 0xb8000 + 160 * 4 ; line 4 (approx)
    call print_string_pm

    ; -------------------------------------------------------------------------
    ; 9. enable pae
    ; -------------------------------------------------------------------------
    mov eax, cr4
    or eax, 1 shl 5
    mov cr4, eax

    ; -------------------------------------------------------------------------
    ; 10. set efer.lme
    ; -------------------------------------------------------------------------
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 shl 8
    wrmsr

    ; -------------------------------------------------------------------------
    ; 11. enable paging
    ; -------------------------------------------------------------------------
    mov eax, cr0
    or eax, 1 shl 31
    mov cr0, eax

    ; -------------------------------------------------------------------------
    ; 12. jump to long mode
    ; -------------------------------------------------------------------------
    jmp 0x18:long_mode_entry

; =============================================================================
; routine: print_string_pm
; input: esi = pointer to string
; input: edi = vga buffer address
; =============================================================================
print_string_pm:
    push eax
    push edi
    push esi
.next_char:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0f    ; white on black
    stosw
    jmp .next_char
.done:
    pop esi
    pop edi
    pop eax
    ret

; =============================================================================
; 64-bit long mode
; =============================================================================
use64
long_mode_entry:
    ; -------------------------------------------------------------------------
    ; 13. print banner (long mode)
    ; -------------------------------------------------------------------------
    mov rsi, banner_lm
    mov rdi, 0xb8000 + 160 * 5 ; line 5
    call print_string_lm

    ; -------------------------------------------------------------------------
    ; 14. halt
    ; -------------------------------------------------------------------------
lm_halt:
    hlt
    jmp lm_halt

; =============================================================================
; routine: print_string_lm
; input: rsi = pointer to string
; input: rdi = vga buffer address
; =============================================================================
print_string_lm:
    push rax
    push rdi
    push rsi
.next_char:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0f    ; white on black
    stosw
    jmp .next_char
.done:
    pop rsi
    pop rdi
    pop rax
    ret

banner_pm db 'stage2: entered protected mode', 0
banner_paging db 'stage2: paging structures initialized', 0
banner_lm db 'stage2: entered long mode', 0
