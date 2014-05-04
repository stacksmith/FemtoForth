X86-32-linux notes

Register usage:

REG             VM              KERNEL          Preserve?
=========================================================
eax             -               TOS
ebx             -               DATA            YES
ecx             -               
edx             tok             scratch         
esi             IP                              YES
edi             vmptr                           YES
ebp             -               DSP             YES
esp             SP              SP              YES


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

;==============================================================================
; error handling
;
; On x86, we are out of registers, so we will keep the error frame in RAM...

;---------------------------------------------
; continue execution after saving the frame...
CODE "core'error'catch // (--0) set up error handling",errset,T_NONE
    DPUSH       eax
    push        esi     ;preserve IP
    push        dword[ERROR_FRAME]
    mov         [ERROR_FRAME],esp
    xor         eax,eax         ;returning 0
    NEXT
.x:    
;---------------------------------------------
; revoke error handler and re-establish previous one
CODE "core'error'clear // (--) restore previous handler",errclr,T_NONE
    mov         esp,[ERROR_FRAME]
    pop         dword[ERROR_FRAME]      ;restore error frame
    add         esp,4                   ;skip IP - we don't need it
    NEXT
.x:    
;---------------------------------------------
; revoke error handler and re-establish previous one
CODE "core'error'throw // (id--) execute active catch, with id",errthrow,T_NONE
    mov         esp,[ERROR_FRAME]       ;restore stack
    pop         dword[ERROR_FRAME]      ;restore error frame
    pop         esi                     ;go to catch
    NEXT
.x:    