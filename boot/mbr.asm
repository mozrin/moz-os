; =============================================================================
; moz-os phase 0001: minimal mbr boot sector
; =============================================================================

format binary
use16
org 0x7c00

start:
    ; -------------------------------------------------------------------------
    ; 1. setup segments
    ; -------------------------------------------------------------------------
    cli                 ; disable interrupts during setup
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00      ; stack grows down from boot sector start
    sti                 ; re-enable interrupts

    ; -------------------------------------------------------------------------
    ; 2. print banner
    ; -------------------------------------------------------------------------
    mov si, banner_msg
    call print_string

    ; -------------------------------------------------------------------------
    ; 3. halt
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
    lodsb               ; load byte from ds:si into al, increment si
    test al, al         ; internal check for null terminator
    jz .done
    mov ah, 0x0e        ; bios teletype function
    int 0x10
    jmp .next_char
.done:
    pop si
    pop ax
    ret

; =============================================================================
; data
; =============================================================================
banner_msg db 'mbr: moz-os boot entry', 13, 10, 0

; =============================================================================
; padding & signature
; =============================================================================
times 510 - ($ - $$) db 0
dw 0xaa55
