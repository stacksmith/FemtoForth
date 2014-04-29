; kernel.asm
;
; used do build the kernel for meow-meow.
;
; format: described in macro below
;
use32

include "../kernel_common.s"
; Register usage:
DSP equ ebp
TOS equ eax


macro RETURN {
    jmp         edi
}
macro DPUSH reg{
    sub         ebp,4           ;DPUSH
    mov         dword[ebp],reg  ;
}
macro DPOP reg {
    mov         reg,[ebp]
    add         ebp,4
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
 ;and in reverse.. interpreter is already on the stack!
    mov         ecx,DWORD $01000000;    ;TODO:*** THIS SUCKS
    
    push        edi                     ;vm pointer
    push        ecx                     ;dat TODO:*** THIS SUCKS
    push        DWORD 0                 ;er
    push        ebp                     ;DSP
    push        esi                     ;IP
    push        eax                     ;TOS
    mov         DWORD[ecx+SP_MEOW],esp  ;save meow stack pointer

    mov         esp,[ecx+SP_C]          ;restore c stack
    pop         edi
    pop         esi
    pop         ebp
    pop         ecx
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
;------------------------------------------------------------------------------
CODE "sys'time ",sys_time,T_NONE
    DPUSH       eax
    pusha                               ;eax,ecx,edx,ebx,?,ebp,esi,edi
    mov         eax,0x0D                ;sys_time
    lea         ebx,[esp+28]            ;eax will return value
    int         0x80
    popa
    
    RETURN
.x:
;------------------------------------------------------------------------------
CODE "sys'gettimeofday (--Sec,uSec)",sys_gettimeofday,T_NONE
    DPUSH       eax
    pusha
    mov         eax,78                  ;sys_timeofday
    lea         ebx,[esp+24]            ;eax
    xor         ecx,ecx                 ;no tz
    int         0x80
    popa
    DPUSH       ecx
    RETURN
.x:

;==============================================================================
; FORTH basics
;------------------------------------------------------------------------------
; else # (--)   unconditional jump to offset
;
CODE "core'DSP (--DSP) get DataStack pointer",DSP,T_NONE
        DPUSH   eax
        mov     eax,DSP
        RETURN
.x:
;------------------------------------------------------------------------------
;
CODE "core'dup (n--n,n)",dup,T_NONE
        DPUSH   eax
        RETURN
.x:
;------------------------------------------------------------------------------
;
CODE "core'drop (n--)",drop,T_NONE
        DPOP    eax
        RETURN
.x:

;------------------------------------------------------------------------------
;
CODE "core'swap (n--)",swap,T_NONE
        xchg    eax,[ebp]
        RETURN
.x:

;------------------------------------------------------------------------------
CODE "core'push (n--) push n onto ReturnStack",push,T_NONE
        push    eax
        DPOP    eax
        RETURN
.x:

;==============================================================================
;------------------------------------------------------------------------------
; U8  (--U8)   load a U8 from codestream.
;
CODE "core'U8 (--n) fetch a U8 that follows in the codestream",U8,T_U8
    DPUSH       eax
    xor         eax,eax
    mov         al,[esi]
    add         esi,1
    RETURN
.x:
;------------------------------------------------------------------------------
; U16 (--U16)  load a U16 from codestream.
;
CODE "core'U16 (--n) fetch a U16 that follows in the codestream",U16,T_U16
    DPUSH       eax
    xor         eax,eax
    mov         ax,[esi]
    add         esi,2
    RETURN
.x:
;------------------------------------------------------------------------------
; U32 (--U32)   load a 32 from codestream.
;
CODE "core'U32 (--n) fetch a U32 that follows in the codestream",U32,T_U32
    DPUSH       eax
    xor         eax,eax
    mov         eax,[esi]
    add         esi,4
    RETURN
.x:
;------------------------------------------------------------------------------
; REF (--REF)   load a reference via table. ***TABLE
;
CODE "core'REF (--n) fetch a REF that follows in the codestream",REF,T_REF
    DPUSH       eax             ;just like U32 fetch
    xor         eax,eax
    mov         al,[esi]
    add         esi,1
    mov         ecx,esi         ;calculate table base
    shr         ecx,4
    shl         ecx,2
    ;
    mov         eax,[ecx+eax*4]
    RETURN
.x:
;==============================================================================
;------------------------------------------------------------------------------
; times
;
; count on return stack.  Loop to offset. Clean up RSP at the end...
CODE "core'times (--) execute expression that follows cnt times",times,T_OFF
        sub         DWORD[esp],1
        jz          .z
        movsx       ebx,byte[esi]
        add         esi,1
        add         esi,ebx
        RETURN
.z:     add         esi,1               ;skip offset
        add         esp,4               ;get rid of the 0 on rsp
        RETURN
.x:
;==============================================================================

;------------------------------------------------------------------------------
CODE "core'op'+ (a,b--sum)",add,T_NONE
    add         eax,[ebp]
    add         ebp,4
    RETURN
.x:
;==============================================================================

;------------------------------------------------------------------------------
CODE "core'op'- (a,b--(a-b))",sub,T_NONE
    mov         ebx,[ebp]       ;ebx = a
    sub         ebx,eax
    add         ebp,4
    mov         eax,ebx
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