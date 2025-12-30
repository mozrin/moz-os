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
    ; 3. load stage 2 (lba read)
    ; -------------------------------------------------------------------------
    ; check for lba extensions if needed, but for now we assume modern bios.
    
    mov ah, 0x42        ; extended read
    mov dl, 0x80        ; first hard disk (usually) - might need to rely on dl from bios
    mov si, disk_packet
    int 0x13
    jc disk_error
    
    ; -------------------------------------------------------------------------
    ; 4. jump to stage 2
    ; -------------------------------------------------------------------------
    jmp 0x0000:0x8000

disk_error:
    mov si, error_msg
    call print_string
    jmp halt_loop

    ; -------------------------------------------------------------------------
    ; 5. halt (fallback)
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
error_msg  db 'disk read error', 13, 10, 0

align 16
disk_packet:
    db 0x10             ; size of packet
    db 0                ; always 0
    dw 1                ; number of sectors to transfer
    dw 0x8000           ; transfer buffer (offset)
    dw 0x0000           ; transfer buffer (segment)
    dq 2                ; starting lba (0-based) 
                        ; lba 0 = mbr, lba 1 = reserved/padding(?), lba 2 = stage 2
                        ; (we will place stage 2 at offset 1024 bytes in the image)

; =============================================================================
; padding & signature
; =============================================================================
times 510 - ($ - $$) db 0
dw 0xaa55
