;X86
;
format elf
use32

section '.text' executable
SP_C       equ 40
SP_MEOW    equ 44

;------------------------------------------------------------------------------
; invoke  the meow-meow interpreter
;
; (tablebase)
public meow_invoke
meow_invoke:

    mov         ecx,[esp+4]             ;get tablebase off the stack
    push        ebx
    push        ebp
    push        esi
    push        edi
    
    mov         DWORD[ecx+SP_C],esp     ;save c stack

    mov         esp,[ecx+SP_MEOW]       ;load meow stack
    pop         eax                     ;TOS
    pop         esi                     ;IP
    pop         ebp                     ;DSP
    add         esp,8                   ;ER
    pop         ebx                     ; interp
 jmp ebx
.x:

;=================================================================================================
; Inner interpreter.
; esi = IP
; eax = interpreter temp
;
;TODO: TOS cannot be eax...
public inner_interpreter
return:
    pop       esi
inner_interpreter:
inner:
    xor       eax,eax               ;clear upper 3 bytes for lodsb
    lodsb                           ;al=tok, inc esi
    shl       eax,2                 ;token*4 (pointers are 4-bytes), set flags...
    jz        return                ;A 0 token that is not first means return
inner_loop:
    push      esi                   ;thread in...
    shr       esi,4                 ;Tricky: esi shr 4 then shl 2 for alignment
    mov       esi,[esi*4+eax]       ;and index it with token*4 resulting in CALL
    xor       eax,eax
    lodsb
    shl       eax,2                 ;first byte of subroutine 0? Machine language code follows
    jnz       inner_loop            ;continue threading
    call      esi                   ;call assembly subroutine that follows
    jmp       return                ;thanks for catching a bug, KSM
.continue:                              ;non-native interpreter
ret
