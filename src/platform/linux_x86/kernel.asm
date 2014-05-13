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
.z1: db str,$a,"CODE",$a,0
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
;------------------------------------------------------------------------------
CODE "system'core'leave // exit to outer host ",leave,T_PROC 
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
CODE "system'core'SYSBASE // (--sysbase) get the bottom of system ",sysbase,T_PROC 
    DPUSH       eax
    mov         eax,ebx
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'invoke // (ptr--) execute ptr via interpreter ",invoke,T_PROC 
    push        esi                     ;save IP, so upon return we will continue
    mov         esi,eax                 ;will execute at ptr
    DPOP        eax                     ;clean up
    NEXT

.x:

;------------------------------------------------------------------------------
CODE "system'core'; // (--) mostly for decompile (usually <0>) ",returno,T_PROC 
    pop         esi
    NEXT;
.x:

;------------------------------------------------------------------------------
; 
CODE "system'TYPE'PU8 // procedure that parses a U8",type_PU8,T_DIR
    db 0;
.x:
;------------------------------------------------------------------------------
; 
CODE "system'TYPE'PU16 // procedure that parses a U16",type_PU16,T_DIR
    db 0;
.x:
;------------------------------------------------------------------------------
; 
CODE "system'TYPE'PU32 // procedure that parses a U32",type_PU32,T_DIR
    db 0;
.x:
;------------------------------------------------------------------------------
CODE "system'TYPE'POFF // procedure that parses an 8-bit offset",type_POFF,T_DIR
    db 0;
.x:
;------------------------------------------------------------------------------
CODE "system'TYPE'PSTR8 // procedure that parses a string",type_PSTR8,T_DIR
    db 0;
.x:
;------------------------------------------------------------------------------
CODE "system'TYPE'PREF // procedure that parses a reference",type_PREF,T_DIR
    db 0;
.x:


;------------------------------------------------------------------------------
CODE "system'io'putc // (c,handle--)",putc1,T_PROC                      ;(c --)
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
CODE "system'io'getc // (handle--c)",getc1,T_PROC                      ;(c --)
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
CODE "system'sys'time",sys_time,T_PROC
    DPUSH       eax
    pusha                               ;eax,ecx,edx,ebx,?,ebp,esi,edi
    mov         eax,0x0D                ;sys_time
    lea         ebx,[esp+28]            ;eax will return value
    int         0x80
    popa
    
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'sys'gettimeofday // (--Sec,uSec)",sys_gettimeofday,T_PROC
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
CODE "system'core'error'catch // (--0) set up error handling",errset,T_PROC
    DPUSH       eax
    push        esi     ;preserve IP (just after catch!)
    push        dword[ebx+ERROR_FRAME]
    mov         [ebx+ERROR_FRAME],esp
    xor         eax,eax         ;returning 0
    NEXT
.x:    
;---------------------------------------------
; revoke error handler and re-establish previous one
CODE "system'core'error'clear // (--) restore previous handler",errclr,T_PROC
    mov         esp,[ebx+ERROR_FRAME]
    pop         dword[ebx+ERROR_FRAME]      ;restore error frame
    add         esp,4                   ;skip IP - we don't need it
    NEXT
.x:    
;---------------------------------------------
; revoke error handler and re-establish previous one
CODE "system'core'error'throw // (id--) execute active catch, with id",errthrow,T_PROC
    mov         esp,[ebx+ERROR_FRAME]       ;restore stack
    pop         dword[ebx+ERROR_FRAME]      ;restore error frame
    pop         esi                     ;go to catch
    NEXT
.x:    
;==============================================================================
; FORTH basics
;------------------------------------------------------------------------------
CODE "system'core'DSP // (--DSP) get the Data Stack Pointer",DSP,T_PROC
        DPUSH   eax
        mov     eax,DSP
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'RSP // (--RSP) get Return Stack Pointer",RSP ,T_PROC
        DPUSH   eax
        mov     eax,esp
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'RSP@ // (--val) value at RSP",ATRSP ,T_PROC
        DPUSH   eax
        mov     eax,[esp]
        NEXT
.x:

;------------------------------------------------------------------------------
;
CODE "system'core'dup // (n -- n n) Duplicates the top stack item.",dup,T_PROC
        DPUSH   eax
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'drop // (n --) Discards the top stack item.",drop,T_PROC
        DPOP    eax
        NEXT
.x:

;------------------------------------------------------------------------------
;
CODE "system'core'swap // (n1 n2 -- n2 n1) Reverses the top two stack items.",swap,T_PROC
        xchg    eax,[ebp]
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'over // (n1 n2 -- n1 n2 n1) Makes a copy of the second item and pushes it on top.",over,T_PROC
        DPUSH   eax
        mov     eax,[ebp+4]
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'rot // (a b c -- b c a) Rotates the third item to the top.",rot,T_PROC
        DPOP    edx             ;edx=n2
        DPOP    ecx             ;ecx=n1
        DPUSH   edx
        DPUSH   eax
        mov     eax,ecx
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'-rot // (a b c -- c a b) rotate the first item to third.",minusrot,T_PROC
        DPOP    edx             ;edx=b
        DPOP    ecx             ;ecx=a
        DPUSH   eax
        DPUSH   ecx
        mov     eax,edx
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'?dup // (a -- a a | 0) duplicate top of stack if non-zero",conddup,T_PROC
        test     eax,eax
        jz      .done
        DPUSH   eax
.done:  NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'dbl'drop // (a b --) Discard 2 items.",dbl_drop,T_PROC
        mov     eax,[ebp+4]
        add     ebp,8
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'dbl'dup // (ab--abab) like OVER OVER.",dbl_dup,T_PROC
        mov     ecx,[ebp]       ;ecx=a  eax=b
        sub     ebp,8           ;room for 2 more
        mov     DWORD[ebp+4],eax     ;ab?b
        mov     [ebp],ecx
        NEXT
.x:

;------------------------------------------------------------------------------
;
CODE "system'core'1+ // (a -- a+1) increment",incr,T_PROC
        inc     eax
.done:  NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'1- // (a -- a-1) decrement",decr,T_PROC
        dec     eax
.done:  NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'4+ // (a -- a+4) increment by 4",incr4,T_PROC
        add     eax,4
.done:  NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'4- // (a -- a-4) decrement by 4",decr4,T_PROC
        sub     eax,4
.done:  NEXT
.x:

;------------------------------------------------------------------------------
;
CODE "system'core'4* // (a -- a*4) mul by 4",mul4,T_PROC
        shl     eax,2
.done:  NEXT
.x:


;------------------------------------------------------------------------------
CODE "system'core'+ // (a,b--sum)",add,T_PROC
    add         eax,[ebp]
    add         ebp,4
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'- // (a,b--(a-b))",sub,T_PROC
    mov         edx,[ebp]       ;edx = a
    sub         edx,eax
    add         ebp,4
    mov         eax,edx
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'* // (a,b--a*b)",mul,T_PROC
    DPOP        edx
    imul        edx
    NEXT
.x:
;==============================================================================
; FORTH comparisons
;------------------------------------------------------------------------------
CODE "system'core'= // (n1 n2 -- flag) True if n1 = n2",cmp_eq,T_PROC
    xor         edx,edx
    cmp         eax,[ebp]
    setz        dl
    add         ebp,4
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'<> // (n1 n2 -- flag) True if n1 <> n2",cmp_ne,T_PROC
    xor         edx,edx
    cmp         eax,[ebp]
    setne        dl
    add         ebp,4
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'< // (n1 n2 -- flag) True if n1 < n2",cmp_lt,T_PROC
    xor         edx,edx
    cmp         [ebp],eax
    setl        dl
    add         ebp,4
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'lt // (n1 n2 --n1, flag) True if n1 < n2",pres_cmp_lt,T_PROC
    xor         edx,edx
    cmp         [ebp],eax
    setl        dl
    mov         eax,edx
    NEXT
    
.x:
;------------------------------------------------------------------------------
CODE "system'core'> // (n1 n2 -- flag) True if n1 > n2",cmp_gt,T_PROC
    xor         edx,edx
    cmp         [ebp],eax
    setg        dl
    add         ebp,4
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'gt // (n1 n2 --n1, flag) True if n1 > n2",pres_cmp_gt,T_PROC
    xor         edx,edx
    cmp         [ebp],eax
    setg        dl
    mov         eax,edx
    NEXT  
.x:
;------------------------------------------------------------------------------
CODE "system'core'<= // (n1 n2 -- flag) True if n1 <= n2",cmp_le,T_PROC
    xor         edx,edx
    cmp         [ebp],eax
    setle       dl
    add         ebp,4
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'le // (n1 n2 --n1, flag) True if n1 <= n2",pres_cmp_le,T_PROC
    xor         edx,edx
    cmp         [ebp],eax
    setle       dl
    mov         eax,edx
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'>= // (n1 n2 -- flag) True if n1 > n2",cmp_ge,T_PROC
    xor         edx,edx
    cmp         [ebp],eax
    setge       dl
    add         ebp,4
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'ge // (n1 n2 --n1, flag) True if n1 <= n2",pres_cmp_ge,T_PROC
    xor         edx,edx
    cmp         [ebp],eax
    setge       dl
    mov         eax,edx
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'0= // (n1 -- flag) True if n1 is 0",cmp_zr,T_PROC
    xor         edx,edx
    cmp         edx,eax
    sete        dl
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'zr // (n1 -- n1,flag) True if n1 is 0",pres_cmp_zr,T_PROC
    DPUSH       eax
    xor         edx,edx
    cmp         eax,edx
    sete        dl
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'0<> // (n1 -- flag) True if n1 is not 0",cmp_nz,T_PROC
    xor         edx,edx
    cmp         edx,eax
    setne       dl
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'nz // (n1 -- n1,flag) True if n1 is not 0",pres_cmp_nz,T_PROC
    DPUSH       eax
    xor         edx,edx
    cmp         edx,eax
    setne       dl
    mov         eax,edx
    NEXT
.x: 

;------------------------------------------------------------------------------
CODE "system'core'0< // (n1 -- flag) True if n1 is less than 0",cmp_ltz,T_PROC
    xor         edx,edx
    test        eax,eax
    setl        dl
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'0> // (n1 -- flag) True if n1 is greater than 0",cmp_gtz,T_PROC
    xor         edx,edx
    test        eax,eax
    setg        dl
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'0<= // (n1 -- flag) True if n1 is less then or equal to 0",cmp_lez,T_PROC
    xor         edx,edx
    test        eax,eax
    setle       dl
    mov         eax,edx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'0>= // (n1 -- flag) True if n1 is greater then or equal to 0",cmp_gez,T_PROC
    xor         edx,edx
    test        eax,eax
    setge       dl
    mov         eax,edx
    NEXT
.x: 
;==============================================================================
;==============================================================================
; special conditionals.  RHS is destroyed, but not lhs!
CODE "system'core'if'> // (n1 n2 -- n1) conditionally execute the following expression",if_gt,T_PROC
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
CODE "system'core'and // (n1 n2 -- n1&n2) logical and",log_and,T_PROC
    and         eax,[ebp]
    add         ebp,4
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'or // (n1 n2 -- n1|n2) logical or",log_or,T_PROC
    or         eax,[ebp]
    add         ebp,4
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'xor // (n1 n2 -- n1^n2) logical xor",log_xor,T_PROC
    xor         eax,[ebp]
    add         ebp,4
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'invert // (n1 -- ~n1) bitwise not",bit_not,T_PROC
    not         eax
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'negate // (n1 -- 0-n1) arithmetic negation",negate,T_PROC
    neg         eax
    NEXT
.x: 

;==============================================================================
; FORTH shifts 
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
CODE "system'core'shl // (n1 n2 -- n1<<n2) shift n1 left by n2 bits",lshift,T_PROC
    mov         ecx,eax
    DPOP        eax
    shl         eax,cl
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'shr // (n1 n2 -- n1>>n2) shift n1 right by n2 bits",rshift,T_PROC
    mov         ecx,eax
    DPOP        eax
    shr         eax,cl
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'ror // (n1 n2 -- n) rotate n1 right by n2 bits",rotr,T_PROC
    mov         ecx,eax
    DPOP        eax
    ror         eax,cl
    NEXT
.x: 
;==============================================================================
; FORTH memory 
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
CODE "system'core'@ // (addr -- val) fetch val from addr",fetch,T_PROC
    mov         eax,[eax]
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'! // (val addr --) store val at addr",store,T_PROC
    mov         edx,[ebp]
    add         ebp,4
    mov         [eax],edx
    DPOP        eax
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'@++ // (addr -- addr+4 val) fetch and increment pointer",finc,T_PROC
        mov     edx,eax
        mov     eax,[edx]
        add     edx,4
        DPUSH   edx
        NEXT
.x:




        
;------------------------------------------------------------------------------
;
CODE "system'core'swap2 // (a,b,c,d--c,d,a,b)",swap2,T_PROC
        xchg    eax,[ebp+4]     ;a,d,c,b
        mov     edx,[ebp]
        xchg    edx,[ebp+8]     ;c,d,a,b
        mov     [ebp],edx
        NEXT
.x:

;------------------------------------------------------------------------------
CODE "system'core'push // (n--) push n onto ReturnStack",push,T_PROC
        push    eax
        DPOP    eax
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'pop // (--n) pop from return stack",pop,T_PROC
        DPUSH   eax
        pop     eax
        NEXT
.x:

;==============================================================================
; Literals (from codestream)
;------------------------------------------------------------------------------
; U8 (--U8) load a U8 from codestream.
;
CODE "system'core'U8 // (--n) fetch a U8 that follows in the codestream",U8,T_U8
    DPUSH       eax
    xor         eax,eax
    mov         al,[esi]
    add         esi,1
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'U8'@ // (addr -- val) fetch a char from addr",cfetch,T_PROC
    movzx       eax,byte[eax]
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'U8'! // (val addr --) store char at addr",cstore,T_PROC
    mov         edx,[ebp]
    add         ebp,4
    mov         [eax],dl
    DPOP        eax
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'U8'@++ // (addr -- addr+1 val) fetch and increment pointer",finc1,T_PROC
        mov     edx,eax
        xor     eax,eax
        mov     al,[edx]
        add     edx,1
        DPUSH   edx
        NEXT
.x:

;------------------------------------------------------------------------------
; U16 (--U16) load a U16 from codestream.
;
CODE "system'core'U16 // (--n) fetch a U16 that follows in the codestream",U16,T_U16
    DPUSH       eax
    xor         eax,eax
    mov         ax,[esi]
    add         esi,2
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'U16'@ // (addr -- val) fetch a 16-bit val from addr",wfetch,T_PROC
    movzx       eax,word[eax]
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'U16'! // (val addr --) store a 16-bit val at addr",wstore,T_PROC
    mov         edx,[ebp]       ;edx =addr
    add         ebp,4
    mov         [eax],dx
    DPOP        eax
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'U16'@++ // (addr -- addr+1 val) fetch and increment pointer",wfinc1,T_PROC
        mov     edx,eax
        xor     eax,eax
        mov     ax,[edx]
        add     edx,2
        DPUSH   edx
        NEXT
.x:

;------------------------------------------------------------------------------
; U32 (--U32) load a 32 from codestream.
;
CODE "system'core'U32 // (--n) fetch a U32 that follows in the codestream",U32,T_U32
    DPUSH       eax
    xor         eax,eax
    mov         eax,[esi]
    add         esi,4
    NEXT
.x:
;------------------------------------------------------------------------------
; REF (--REF) load a reference via table. ***TABLE
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
CODE "system'core'STR8'eq // (str1,cnt1,str2,cnt2--str1,cnt1,flg) compare 2 cstrings",str_cmp,T_PROC
    cmp         eax,[ebp+4]     ;compare counts
    jne         .no
   push        esi
   mov         esi,[ebp+8]      ;esi=str1;
   push        edi
   mov         edi,[ebp]        ;edi=str2;
    mov         ecx,eax         ;count
    xor         eax,eax
    repe cmpsb
    sete       al
   add         ebp,4
   pop         edi
   pop         esi
   NEXT
.no:
   xor         eax,eax
   add         ebp,4
   pop         edi
   pop         esi
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
; limit start do ... loop
;
; return stack will contain: (--addr,limit,counter)
CODE "system'core'do // (limit,start--) set up a counted loop",_do,T_PROC
    push        esi             ;rstack the loop address
    push        dword [ebp]     ;rstack the limit
    push        eax             ;stack counter...
    add         ebp,4           ;clean up
    DPOP        eax
    NEXT
.x:
CODE "system'core'loop // (--) count and proceed to do site",_loop,T_PROC
    mov         ecx,[esp]       ;counter
    inc         ecx
    cmp         [esp+4],ecx     ;compare with limit
    jle         .over
    mov         [esp],ecx       ;update count
    mov         esi,[esp+8]     ;and loop again.
    NEXT
.over:
    add         sp,12          ;get rid of rstack data
    NEXT
.x:
CODE "system'core'i // (--i) inside a do..loop, return index",_i,T_PROC
    DPUSH       eax
    mov         eax,[esp]
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
CODE "system'core'D- // (ah,al,bh,bl--ch,cl)",2sub,T_PROC
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
CODE "test'dbase ",dbase,T_PROC
    DPUSH eax
    mov eax,ebx
    NEXT
.x:

;------------------------------------------------------------------------------
CODE "test'nop ",nop,T_PROC
    sub         ebp,4
    mov         [ebp],eax
    mov         eax,$DEADDEAD
    NEXT
.x:
;==============================================================================
; variable
;------------------------------------------------------------------------------
CODE "system'TYPE'U32'fetch // (--val)",var_fetchp,T_PROC
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
CODE "system'TYPE'U32'into // (val--)",var_storep,T_PROC
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
CODE "system'TYPE'SYSVAR'prim'compile // (--val)",sysvar_fetchp,T_PROC
    DPUSH       eax          
     xor         eax,eax
     mov         al,[esi]        ;next token
     add         esi,1
    mov         eax,[ebx+eax*4]  ;load value
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'TYPE'SYSVAR'prim'into // (val--)",sysvar_storep,T_PROC
     xor         ecx,ecx
     mov         cl,[esi]        ;next token
     add         esi,1
    mov         [ebx+ecx*4],eax  ;store
    DPOP        eax
    NEXT
.x:

db 0    ;an empty record to terminate load process





