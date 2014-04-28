; kernel.asm
;
; used do build the kernel for meow-meow.
;
; format: described in macro below
;
use32

DATA_BASE  equ 0
DATA_TOP   equ 4
TABLE_BASE equ 8
TABLE_TOP  equ 12
DSP_BASE   equ 16
DSP_TOP    equ 20
RSP_BASE   equ 24
RSP_TOP    equ 28

DATA_PTR   equ 32
RUN_PTR  equ 36
RUN_PTR    equ 40
SP_C       equ 40
SP_MEOW    equ 44
; Register usage:


T_NONE  equ 0
T_U8    equ 1
T_U16   equ 2
T_U32   equ 3
T_OFF   equ 4
T_STR   equ 5

TYPE_PROC equ 3

macro RETURN {
    jmp         edi
}
; Format:
; 1 cnt   count of string, including null-term and padding
; ? name
; 1 parm  - tokenstream data 
;
; The loader will prefix each code word with a 0, properly aligned!
macro CODE str,name,parm {
  db .z2-.z1
.z1: db str,0
align 4
.z2:
; 
db parm
db 0,0,0
dd __#name#.x - __#name         ;DANGER: x86 d is 4 bytes,w is 2!
__#name:
    ;RPOP IP
}
; return to C
CODE "core'leave exit to outer host ",leave,T_NONE 
    ;we are not going back to caller, or interpreter
    add         esp,4
    ;and in reverse.. interpreter is already on the stack!
    push        edi                     ;vm pointer
    push        ecx                     ;dat
    push        DWORD 0                 ;er
    push        ebp                     ;DSP
    push        esi                     ;IP
    push        eax                     ;TOS
    mov         DWORD[ecx+SP_MEOW],esp  ;save meow stack pointer

    mov         esp,[ecx+SP_C]          ;restore c stack
    pop         edi
    pop         esi
    pop         ebp
    pop         ebx

    ret
.x:
;------------------------------------------------------------------------------
CODE "io'emit (c--)",emit,T_NONE                      ;(c -- )
    pusha
    mov         eax,4                   ;fwrite
    mov         ebx,1                   ;handle
    lea         ecx,[esp+28]
    mov         edx,1
    int         0x80
    popa    
    mov         eax,[ebp]
    add         ebp,4
    RETURN
.x:
;==============================================================================
;------------------------------------------------------------------------------
; U8  (--U8)   load a U8 from codestream.
;
CODE "core'U8 (--n) fetch a U8 that follows in the codestream",U8,T_U8
    mov         esi,[esp]             ;src ptr
    xchg        ebp,esp
    push        eax
    xor         eax,eax
    xchg        esp,ebp
    lodsb
    mov         [esp],esi
    RETURN
.x:
;------------------------------------------------------------------------------
; U16 (--U16)  load a U16 from codestream.
;
CODE "core'U16 (--n) fetch a U16 that follows in the codestream",U16,T_U16
    mov         esi,[esp]             ;src ptr
    xchg        ebp,esp
    push        eax
    xor         eax,eax
    xchg        esp,ebp
    lodsw
    mov         [esp],esi
    RETURN
.x:
;------------------------------------------------------------------------------
; U32 (--U32)   load a 32 from codestream.
;
CODE "core'U32 (--n) fetch a U32 that follows in the codestream",U32,T_U32
    mov         esi,[esp]             ;src ptr
    xchg        ebp,esp
    push        eax
    xchg        esp,ebp
    lodsd
    mov         [esp],esi
    RETURN
.x:

;==============================================================================
;------------------------------------------------------------------------------
CODE "core'+ (a,b--sum)",add,T_NONE
    add         eax,[ebp]
    add         ebp,4
    RETURN
.x:


;------------------------------------------------------------------------------
CODE "io'nop ",ionop,T_NONE
    sub         ebp,4
    mov         [ebp],eax
    mov         eax,$DEADDEAD
    ret
    RETURN
.x:
;------------------------------------------------------------------------------
CODE "test'nop ",nop,T_NONE
    sub         ebp,4
    mov         [ebp],eax
    mov         eax,$DEADDEAD
    RETURN
 rept 300 { db 0 } 
.x:

;------------------------------------------------------------------------------
db 0    ;an empty record to terminate load process