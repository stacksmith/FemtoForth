;******************************************************************************
; Copyright 2014 Victor Yurkovsky
;
; This file is part of the FemtoForth project.
;
; FemtoForth is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; FemtoForth is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with FemtoForth. If not, see <http://www.gnu.org/licenses/>.
;
;*****************************************************************************
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


macro NEXT {
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
CODE "system'core'leave // exit to outer host ",leave,T_NONE 
 ;and in reverse.. interpreter is already on the stack!
    
    push        edi                     ;vm pointer
    push        ecx                     ;
    push        DWORD 0                 ;er
    push        ebp                     ;DSP
    push        esi                     ;IP
    push        eax                     ;TOS
    mov         DWORD[ebx+SP_MEOW],esp  ;save meow stack pointer

    mov         esp,[ebx+SP_C]          ;restore c stack
    pop         edi
    pop         esi
    pop         ebp
    pop         ecx
    pop         ebx

    ret

.x:
;------------------------------------------------------------------------------
CODE "system'io'putc // (c,handle--)",putc1,T_NONE                      ;(c --)
    pusha
    mov         eax,4                   ;fwrite
    mov         ebx,[esp+28]            ;handle in TOS
    mov         ecx,ebp                 ;buffer from NOS
    mov         edx,1
    int         0x80
    popa    
    mov         eax,[ebp+4]
    add         ebp,8
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'io'getc // (handle--c)",getc1,T_NONE                      ;(c --)
    DPUSH       eax
    pusha
    mov         eax,3                   ;fread
    mov         ebx,[esp+28]            ;handle in TOS
    lea         ecx,[esp+28]
    mov         edx,1
    int         0x80
    popa   
    and         eax,$FF
    NEXT
.x:

;------------------------------------------------------------------------------
CODE "system'sys'time",sys_time,T_NONE
    DPUSH       eax
    pusha                               ;eax,ecx,edx,ebx,?,ebp,esi,edi
    mov         eax,0x0D                ;sys_time
    lea         ebx,[esp+28]            ;eax will return value
    int         0x80
    popa
    
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'sys'gettimeofday // (--Sec,uSec)",sys_gettimeofday,T_NONE
    DPUSH       eax
    pusha
    mov         eax,78                  ;sys_timeofday
    lea         ebx,[esp+24]            ;eax
    xor         ecx,ecx                 ;no tz
    int         0x80
    popa
    DPUSH       ecx
    NEXT
.x:
;==============================================================================
; error handling
;
; On x86, we are out of registers, so we will keep the error frame in RAM...

;---------------------------------------------
; continue execution after saving the frame...
CODE "system'core'error'catch // (--0) set up error handling",errset,T_NONE
    DPUSH       eax
    push        esi     ;preserve IP (just after catch!)
    push        dword[ebx+ERROR_FRAME]
    mov         [ebx+ERROR_FRAME],esp
    xor         eax,eax         ;returning 0
    NEXT
.x:    
;---------------------------------------------
; revoke error handler and re-establish previous one
CODE "system'core'error'clear // (--) restore previous handler",errclr,T_NONE
    mov         esp,[ebx+ERROR_FRAME]
    pop         dword[ebx+ERROR_FRAME]      ;restore error frame
    add         esp,4                   ;skip IP - we don't need it
    NEXT
.x:    
;---------------------------------------------
; revoke error handler and re-establish previous one
CODE "system'core'error'throw // (id--) execute active catch, with id",errthrow,T_NONE
    mov         esp,[ebx+ERROR_FRAME]       ;restore stack
    pop         dword[ebx+ERROR_FRAME]      ;restore error frame
    pop         esi                     ;go to catch
    NEXT
.x:    
;==============================================================================
; FORTH basics
;------------------------------------------------------------------------------
CODE "system'core'DSP // (--DSP) get the Data Stack Pointer",DSP,T_NONE
        DPUSH   eax
        mov     eax,DSP
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'RSP // (--RSP) get Return Stack Pointer",RSP ,T_NONE
        DPUSH   eax
        mov     eax,esp
        NEXT
.x:

;------------------------------------------------------------------------------
;
CODE "system'core'dup // (n -- n n) Duplicates the top stack item.",dup,T_NONE
        DPUSH   eax
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'drop // (n --) Discards the top stack item.",drop,T_NONE
        DPOP    eax
        NEXT
.x:

;------------------------------------------------------------------------------
;
CODE "system'core'swap // (n1 n2 -- n2 n1) Reverses the top two stack items.",swap,T_NONE
        xchg    eax,[ebp]
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'over // (n1 n2 -- n1 n2 n1) Makes a copy of the second item and pushes it on top.",over,T_NONE
        DPUSH   eax
        mov     eax,[ebp+4]
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'rot // (a b c -- b c a) Rotates the third item to the top.",rot,T_NONE
        DPOP    edx             ;edx=n2
        DPOP    ecx             ;ecx=n1
        DPUSH   edx
        DPUSH   eax
        mov     eax,ecx
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'-rot // (a b c -- c a b) rotate the first item to third.",minusrot,T_NONE
        DPOP    edx             ;edx=b
        DPOP    ecx             ;ecx=a
        DPUSH   eax
        DPUSH   ecx
        mov     eax,edx
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'?dup // (a -- a a | 0) duplicate top of stack if non-zero",conddup,T_NONE
        test     eax,eax
        jz      .done
        DPUSH   eax
.done:  NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'1+ // (a -- a+1) increment",incr,T_NONE
        inc     eax
.done:  NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'1- // (a -- a-1) decrement",decr,T_NONE
        dec     eax
.done:  NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'4+ // (a -- a+4) increment by 4",incr4,T_NONE
        add     eax,4
.done:  NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'4- // (a -- a-14) decrement by 4",decr4,T_NONE
        sub     eax,4
.done:  NEXT
.x:

;------------------------------------------------------------------------------
CODE "system'core'+ // (a,b--sum)",add,T_NONE
    add         eax,[ebp]
    add         ebp,4
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'- // (a,b--(a-b))",sub,T_NONE
    mov         edx,[ebp]       ;edx = a
    sub         edx,eax
    add         ebp,4
    mov         eax,edx
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'* // (a,b--a*b)",mul,T_NONE
    DPOP        edx
    imul        edx
    NEXT
.x:
;==============================================================================
; FORTH comparisons
;------------------------------------------------------------------------------
CODE "system'core'= // (n1 n2 -- flag)   True if n1 = n2",cmp_eq,T_NONE
    xor         edx,edx
    cmp         eax,[ebp]
    setz        dl
    add         ebp,4
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'<> // (n1 n2 -- flag)  \ True if n1 <> n2",cmp_ne,T_NONE
    xor         edx,edx
    cmp         eax,[ebp]
    setne        dl
    add         ebp,4
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'< // (n1 n2 -- flag)  \ True if n1 < n2",cmp_lt,T_NONE
    xor         edx,edx
    cmp         [ebp],eax
    setl        dl
    add         ebp,4
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'> // (n1 n2 -- flag)  \ True if n1 > n2",cmp_gt,T_NONE
    xor         edx,edx
    cmp         [ebp],eax
    setg        dl
    add         ebp,4
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'<= // (n1 n2 -- flag)  \ True if n1 <= n2",cmp_le,T_NONE
    xor         edx,edx
    cmp         [ebp],eax
    setle       dl
    add         ebp,4
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'>= // (n1 n2 -- flag)  \ True if n1 > n2",cmp_ge,T_NONE
    xor         edx,edx
    cmp         [ebp],eax
    setge       dl
    add         ebp,4
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'0= // (n1 -- flag)  \ True if n1 is 0",cmp_zr,T_NONE
    xor         edx,edx
    cmp         edx,eax
    sete        dl
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'0<> // (n1 -- flag)  \ True if n1 is not 0",cmp_nz,T_NONE
    xor         edx,edx
    cmp         edx,eax
    setne       dl
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'0< // (n1 -- flag)  \ True if n1 is less than 0",cmp_ltz,T_NONE
    xor         edx,edx
    test        eax,eax
    setl        dl
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'0> // (n1 -- flag)  \ True if n1 is greater than 0",cmp_gtz,T_NONE
    xor         edx,edx
    test        eax,eax
    setg        dl
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'0<= // (n1 -- flag)  \ True if n1 is less then or equal to 0",cmp_lez,T_NONE
    xor         edx,edx
    test        eax,eax
    setle       dl
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'0>= // (n1 -- flag)  \ True if n1 is greater then or equal to 0",cmp_gez,T_NONE
    xor         edx,edx
    test        eax,eax
    setge       dl
    mov         eax,edx
    NEXT
.x: 
;==============================================================================
;==============================================================================
; special conditionals.  RHS is destroyed, but not lhs!
CODE "system'core'if'> // (n1 n2 -- n1) conditionally execute the following expression",if_gt,T_NONE
    xor         ecx,ecx                 ;offset
    cmp         [ebp],eax               ;compare n1,n2
     setle       cl                      ;if n1 <= n2
    movsx       edx,byte[esi]           ;edx is offset
     sub         ecx,1                   ;if n1 <= n2, 0.  if n1>n2, 0xFFFF
    add         esi,1
     and         edx,ecx
 ;   add         esi,edx                 ;add offset or 0
    DPOP        eax
    NEXT
.x:
;==============================================================================
; FORTH logical 
;------------------------------------------------------------------------------
CODE "system'core'and // (n1 n2 -- n1&n2)  \ logical and",log_and,T_NONE
    and         eax,[ebp]
    add         ebp,4
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'or // (n1 n2 -- n1|n2)  \ logical or",log_or,T_NONE
    or         eax,[ebp]
    add         ebp,4
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'xor // (n1 n2 -- n1^n2)  \ logical xor",log_xor,T_NONE
    xor         eax,[ebp]
    add         ebp,4
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'invert // (n1 -- ~n1)  \ bitwise not",bit_not,T_NONE
    not         eax
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'negate // (n1 -- 0-n1)  \ arithmetic negation",negate,T_NONE
    neg         eax
    NEXT
.x: 

;==============================================================================
; FORTH shifts 
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
CODE "system'core'<< // (n1 n2 -- n1<<n2)  \ shift n1 left by n2 bits",lshift,T_NONE
    mov         ecx,eax
    DPOP        eax
    shl         eax,cl
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'>> // (n1 n2 -- n1>>n2)  \ shift n1 right by n2 bits",rshift,T_NONE
    mov         ecx,eax
    DPOP        eax
    shr         eax,cl
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'ror // (n1 n2 -- n)  \ rotate n1 right by n2 bits",rotr,T_NONE
    mov         ecx,eax
    DPOP        eax
    ror         eax,cl
    NEXT
.x: 
;==============================================================================
; FORTH memory 
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
CODE "system'core'@ // (addr -- val)  \ fetch val from addr",fetch,T_NONE
    mov         eax,[eax]
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'! // (val addr --)  \ store val at addr",store,T_NONE
    mov         edx,[ebp]
    add         ebp,4
    mov         [eax],edx
    DPOP        eax
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'c@ // (addr -- val)  \ fetch val from addr",cfetch,T_NONE
    mov         al,[eax]
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'c! // (val addr --)  \ store val at addr",cstore,T_NONE
    mov         edx,[ebp]
    add         ebp,4
    mov         [eax],dl
    DPOP        eax
    NEXT
.x: 

;------------------------------------------------------------------------------
CODE "system'core'c@++ // (addr -- addr+1 val) fetch and increment pointer",finc1,T_NONE
        mov     edx,eax
        xor     eax,eax
        mov     al,[edx]
        add     edx,1
        DPUSH   edx
        NEXT
.x:
        
;------------------------------------------------------------------------------
;
CODE "system'core'swap2 // (a,b,c,d--c,d,a,b)",swap2,T_NONE
        xchg    eax,[ebp+4]     ;a,d,c,b
        mov     edx,[ebp]
        xchg    edx,[ebp+8]     ;c,d,a,b
        mov     [ebp],edx
        NEXT
.x:

;------------------------------------------------------------------------------
CODE "system'core'push // (n--) push n onto ReturnStack",push,T_NONE
        push    eax
        DPOP    eax
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'pop // (--n) pop from return stack",pop,T_NONE
        DPUSH   eax
        pop     eax
        NEXT
.x:

;==============================================================================
; Literals (from codestream)
;------------------------------------------------------------------------------
; U8 (--U8)   load a U8 from codestream.
;
CODE "system'core'U8 // (--n) fetch a U8 that follows in the codestream",U8,T_U8
    DPUSH       eax
    xor         eax,eax
    mov         al,[esi]
    add         esi,1
    NEXT
.x:
;------------------------------------------------------------------------------
; U16 (--U16)  load a U16 from codestream.
;
CODE "system'core'U16 // (--n) fetch a U16 that follows in the codestream",U16,T_U16
    DPUSH       eax
    xor         eax,eax
    mov         ax,[esi]
    add         esi,2
    NEXT
.x:
;------------------------------------------------------------------------------
; U32 (--U32)   load a 32 from codestream.
;
CODE "system'core'U32 // (--n) fetch a U32 that follows in the codestream",U32,T_U32
    DPUSH       eax
    xor         eax,eax
    mov         eax,[esi]
    add         esi,4
    NEXT
.x:
;------------------------------------------------------------------------------
; REF (--REF)   load a reference via table. ***TABLE
;
CODE "system'core'REF // (--n) fetch a REF that follows in the codestream",REF,T_REF
    DPUSH       eax             ;just like U32 fetch
    xor         eax,eax
    mov         al,[esi]
    add         esi,1
    mov         ecx,esi         ;calculate table base
    shr         ecx,4
    shl         ecx,2
    ;
    mov         eax,[ecx+eax*4]
    NEXT
.x:
;------------------------------------------------------------------------------
; STRING  
;
CODE "system'core'STR8 // (--str,cnt) fetch a string pointer.  String follows inline",STR8,T_STR8
    DPUSH       eax             ;preserve tos
     xor         eax,eax        ;load count into TOS
     mov         al,[esi]
     add         esi,1
    DPUSH       esi             ;STR
    add         esi,eax         ;skip string
    NEXT
 .x:   


;------------------------------------------------------------------------------
; branch
;
CODE "system'core'branch // (--) branch by signed U8 offset",branchU8,T_OFF
    movsx       edx,byte[esi]
    add         esi,1
    add         esi,edx
    NEXT
.x:    
;condition-code 0BRANCH OFFSET true-part rest-code
CODE "system'core'0branch // (cond--) if 0, branch by signed U8 offset",zbranchU8,T_OFF
    movsx       edx,byte[esi]           ;edx is offset
    add         esi,1
    and         eax,1
    dec         eax                     ;0->FFFFFFFF, 1->0
    and         edx,eax
    add         esi,edx                 ;add offset or 0
    DPOP        eax
    NEXT
.x:
;------------------------------------------------------------------------------
; times
;
; count on return stack.  Loop to offset. Clean up RSP at the end...
CODE "system'core'times // (--) execute expression that follows cnt times",times,T_OFF
        sub         DWORD[esp],1
        jz          .z
        movsx       edx,byte[esi]
        add         esi,1
        add         esi,edx
        NEXT
.z:     add         esi,1               ;skip offset
        add         esp,4               ;get rid of the 0 on rsp
        NEXT
.x:


;==============================================================================

;------------------------------------------------------------------------------
CODE "system'core'D- // (ah,al,bh,bl--ch,cl)",2sub,T_NONE
    mov         edx,[ebp+4]       ;edx = al
    sub         edx,eax
    mov         eax,edx           ;low done
    mov         edx,[ebp+8]       ;edx = ah
    sbb         edx,[ebp]
    add         ebp,12    
    mov         [ebp],edx
    NEXT
.x:

;------------------------------------------------------------------------------
CODE "test'dbase ",dbase,T_NONE
    DPUSH eax
    mov eax,ebx
    NEXT
.x:

;------------------------------------------------------------------------------
CODE "test'nop ",nop,T_NONE
    sub         ebp,4
    mov         [ebp],eax
    mov         eax,$DEADDEAD
    NEXT
.x:
;==============================================================================
; variable
;------------------------------------------------------------------------------
CODE "system'TYPE'U32'fetch // (--val)",var_fetchp,T_NONE
    DPUSH       eax          
     xor         eax,eax
     mov         al,[esi]        ;next token
     add         esi,1
    mov         ecx,esi         ;calculate table base ***
    shr         ecx,4
    shl         ecx,2
    ;
    mov         eax,[ecx+eax*4]
    mov         eax,[eax]
    NEXT
.x:
;==============================================================================
; store variable
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
CODE "system'TYPE'U32'into // (val--)",var_storep,T_NONE
     xor         edx,edx
     mov         dl,[esi]        ;next token
     add         esi,1
    mov         ecx,esi         ;calculate table base ***
    shr         ecx,4
    shl         ecx,2
    ;
    mov         edx,[ecx+edx*4]         ;address of variable
    mov         [edx],eax
    DPOP        eax
    NEXT
.x:
;==============================================================================
; sysvar
;
; Sysvars are 32-bit variables inside the dictionary.  Since it's relocatable,
; we cannot rely on absolute addresses, and offset from a runtime base value
; passed to us id ebx.
;
; sysvar types stored as indices (system multiplies them by 4 to access)
;
; sysvars are compiled as <sysvar'prim'compile><index> for optimal speed
;------------------------------------------------------------------------------
CODE "system'TYPE'SYSVAR'prim'compile // (--val)",sysvar_fetchp,T_NONE
    DPUSH       eax          
     xor         eax,eax
     mov         al,[esi]        ;next token
     add         esi,1
    mov         eax,[ebx+eax*4]  ;load value
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'TYPE'SYSVAR'prim'into // (val--)",sysvar_storep,T_NONE
     xor         ecx,ecx
     mov         cl,[esi]        ;next token
     add         esi,1
    mov         [ebx+ecx*4],eax  ;store
    DPOP        eax
    NEXT
.x:
;------------------------------------------------------------------------------
db 0    ;an empty record to terminate load process