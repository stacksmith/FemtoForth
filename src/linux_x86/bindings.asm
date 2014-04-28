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
    ;preserve C registers
    push        ebx
    push        ecx
    push        ebp
    push        esi
    push        edi
    
    mov         DWORD[ecx+SP_C],esp     ;save c stack
    ;load meow registers from structure passed to this function.
    ;I am thinking of multithreading etc.
    mov         esp,[ecx+SP_MEOW]       ;load meow stack
    pop         eax                     ;TOS
    pop         esi                     ;IP
    pop         ebp                     ;DSP
    add         esp,8                   ;ER
    pop         edi                     ;EDI=interp
    ; invoke
   jmp         edi

;and in reverse.. interpreter is already on the stack!
    push        edi                     ;vm pointer
    push        DWORD $01000000         ;dat TODO:*** THIS SUCKS
    push        DWORD 0                 ;er
    push        ebp                     ;DSP
    push        esi                     ;IP
    push        eax                     ;TOS
    mov         DWORD[ecx+SP_MEOW],esp  ;save meow stack pointer

    mov         ecx,DWORD $01000000;    ;TODO:*** THIS SUCKS
    mov         esp,[ecx+SP_C]          ;restore c stack
    pop         edi
    pop         esi
    pop         ebp
    pop         ecx
    pop         ebx

    ret
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
    xor       edx,edx               ;clear upper 3 bytes for lodsb
    mov       dl,[esi]              ;al=tok, inc esi
    inc       esi
    shl       edx,2                 ;token*4 (pointers are 4-bytes), set flags...
    jz        return                ;A 0 token that is not first means return
.inner_loop:
    push      esi                   ;thread in...
    shr       esi,4                 ;Tricky: esi shr 4 then shl 2 for alignment
    mov       esi,[esi*4+edx]       ;and index it with token*4 resulting in CALL
    xor       edx,edx
    mov       dl,[esi]              ;al=tok, inc esi
    inc       esi
    shl       edx,2                 ;first byte of subroutine 0? Machine language code follows
    jnz       .inner_loop            ;continue threading
    mov       ecx,esi               ;routine address
    pop       esi                   ;no stacking of IP...
    jmp       ecx                   ;call assembly subroutine that follows
