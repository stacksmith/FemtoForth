X86-32-linux notes

Register usage:

REG             VM              KERNEL          DESC
=========================================================
eax             -               TOS
ebx             -                
ecx             -               scratch
edx             tok             scratch         
esi             IP              scratch     
edi             vm              
ebp             -               DSP
esp             SP              SP


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
    call      esi                   ;call assembly subroutine that follows
    jmp       return                ;thanks for catching a bug, KSM
