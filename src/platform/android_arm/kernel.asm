;******************************************************************************
; Copyright 2014 Victor Yurkovsky
;
; This file is part of the FemtoForth project.
;
; FPGAsm is free software: you can redistribute it and/or modify
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

include "../kernel_common.s"

; Register usage:

ERR equ r5

IP equ r6
IP! equ r6!

DSP equ r7
DSP! equ r7!

ER equ r9
ER! equ r9!

RDAT equ r11            ;data segment register...

RSP equ sp
RSP! equ sp!

TOS equ r0

macro RPUSH reg {
  str reg,[RSP,-4]!
}
macro RPOP reg {
  ldr reg,[RSP],4
}

macro DPUSH reg {
  str reg,[DSP,-4]!
}
 
macro DPOP reg {
  ldr reg,[DSP],4
}

macro NEXT {
    bx lr
}
; return to C
CODE "system'core'leave exit to outer host ",leave,T_NONE 
        push    {r0,r6,r7,r9,r11,lr}
        str     sp,[RDAT,SP_MEOW]                  ;consider not storing for reentrancy
        ldr     sp,[RDAT,SP_C]
        pop     {r4-r11,lr}
        bx      lr
.x:
CODE "system'core'nop ",nop,T_NONE 
mov r0,0xDEAD
        bx      lr
.x:

CODE "test'a",testa,T_NONE 

 mov r1,'a'
        push    {r0-r7,r11,lr}
        mov     r0,1                            ;stdout
        add     r1,RSP,4                        ;char is RSP[4]
        mov     r2,1
        mov     r7,4                            ;write
        swi     0
        pop     {r0-r7,r11,lr}
        bx      lr
.x:  
CODE "test'b",testb,T_NONE 
 mov r0,'b'
        push    {r0-r7,r11,lr}
        mov     r0,1                            ;stdout
        add     r1,RSP,0                        ;char is RSP[4]
        mov     r2,1
        mov     r7,4                            ;write
        swi     0
        pop     {r0-r7,r11,lr}
        mov r0,DSP
        bx      lr
.x:  
        

CODE "system'system'irp1",irp1,T_NONE
.1: ldrb    r12,[IP],1               ;2; fetch a token
        lsls    r12,2                    ;1; r3 = table index; set Z if code.
        bxeq    IP                      ;2; 0=CODE! 
        RPUSH   IP
        lsr     IP,IP,4                 ;1; create a 16-byte range address
        ldr     IP,[r12,IP,LSL 2]        ;2; r2 is return IP // (dereference table)
        b       .1                  ;1;
; Observations:
; lr must be set to innerl.  Subroutines must preserve AND RESTORE lr!
.x:

;------------------------------------------------------------------------------
;CODE "system'core'; // (--) return",return,T_NONE
;    RPOP        IP
;;mov r0,0xDEAD
;    bx          lr
;.x:
;------------------------------------------------------------------------------
CODE "system'io'putc // (c,handle--)",putc1,T_NONE                      ;(c --)
        push    {r0-r7,lr}
;       ldr     r0,1                            ;r0 is already handle..
        mov     r1,DSP                          ;char is RSP[0], saved r0
        mov     r2,1
        mov     r7,4                            ;write
        swi     0
        pop     {r0-r7,lr}
        add     DSP,4
        ldr     r0,[DSP],4                      ;drop
        bx      lr
.x:  
;------------------------------------------------------------------------------
CODE "system'io'getc // (handle--c)",getc1,T_NONE                      ;(c --)
        DPUSH   r0
        push    {r0-r7,lr}
        mov     r0,0                            ;stdout
        mov     r1,RSP                          ;char is RSP[0],r0
        mov     r2,1
        mov     r7,3                            ;fread
        swi     0
        pop     {r0-r7,lr}
        and     r0,$FF
        bx      lr
.x:
;------------------------------------------------------------------------------
CODE "system'sys'gettimeofday // (--Sec,uSec)",sys_gettimeofday,T_NONE
        DPUSH   r0
        push    {r0-r7,lr}
        mov     r0,RSP                          ;will return data in r0,r1
        mov     r1,0
        mov     r7,78                         ;gettimeofday
        swi     0
        pop     {r0-r7,lr}
        DPUSH   r0
        mov     r0,r1
        NEXT
.x:
;==============================================================================
; error handling
;
; On x86, we are out of registers, so we will keep the error frame in RAM...

;---------------------------------------------
; continue execution after saving the frame...
CODE "system'core'error'catch // (--0) set up error handling",errset,T_NONE
        DPUSH   r0
        RPUSH   IP              ;preserve IP (just after catch!)
        RPUSH   ERR             ;preserve error frame
        mov     ERR,sp          ;and frame it
        mov     r0,0            ;returning 0
        NEXT
.x:    
;---------------------------------------------
; revoke error handler and re-establish previous one
CODE "system'core'error'clear // (--) restore previous handler",errclr,T_NONE
        mov     sp,ERR          ;magically restore stack pointer
        ldr     ERR,[sp],8      ;and previous error frame; skip IP
        NEXT
.x:    
;---------------------------------------------
; revoke error handler and re-establish previous one
CODE "system'core'error'throw // (id--) execute active catch, with id",errthrow,T_NONE
        mov     sp,ERR          ;magically restore stack pointer
        RPOP    ERR             ;previous error
        RPOP    IP              ;and prepare to reenter
        NEXT
.x:  
;==============================================================================
; FORTH basics
;------------------------------------------------------------------------------
CODE "system'core'DSP // (--DSP) get the Data Stack Pointer",DSP,T_NONE
        DPUSH   r0
        mov     r0,DSP
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'RSP // (--RSP) get Return Stack Pointer",RSP ,T_NONE
        DPUSH   r0
        mov     r0,RSP
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'dup // (n -- n n) Duplicates the top stack item.",dup,T_NONE
        DPUSH   r0
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'drop // (n --) Discards the top stack item.",drop,T_NONE
        DPOP    r0
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'swap // (n1 n2 -- n2 n1) Reverses the top two stack items.",swap,T_NONE
        swp     r0,r0,[DSP]
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'over // (n1 n2 -- n1 n2 n1) Makes a copy of the second item and pushes it on top.",over,T_NONE
        DPUSH   r0
        ldr     r0,[DSP,4]
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'rot // (a b c -- b c a) Rotates the third item to the top.",rot,T_NONE
        DPOP    r1              ;r1=b
        DPOP    r2              ;r2=a
        DPUSH   r1
        DPUSH   r0
        mov     r0,r2
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'-rot // (a b c -- c a b) Rotates the first item to third.",minusrot,T_NONE
        DPOP    r1              ;r1=b
        DPOP    r2              ;r2=a
        DPUSH   r0
        DPUSH   r2
        mov     r0,r1
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'?dup // (a -- a a | 0) duplicate top of stack if non-zero",conddup,T_NONE
        cmp     r0,0
        streq   r0,[DSP,-4]!
.done:  NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'1+ // (a -- a+1) increment",incr,T_NONE
        add     r0,1
.done:  NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'1- // (a -- a-1) decrement",decr,T_NONE
        sub     r0,1
.done:  NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'4+ // (a -- a+4) increment by 4",incr4,T_NONE
        add     r0,4
.done:  NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "system'core'4- // (a -- a-14) decrement by 4",decr4,T_NONE
        sub     r0,4
.done:  NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'+ // (a,b--sum)",add,T_NONE
        DPOP    r1
        add     r0,r1
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'- // (a,b--(a-b))",sub,T_NONE
        DPOP    r1                      ;r1 = a
        sub     r0,r1,r0
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'* // (a,b--a*b)",mul,T_NONE
        DPOP    r1
        SMULL   r0,r1,r0,r1
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'2* // (n--n*2)",mul2,T_NONE
        lsl     r0,1
        NEXT
.x:

;------------------------------------------------------------------------------
CODE "system'core'= // (n1 n2 -- flag)  \ True if n1 = n2",cmp_eq,T_NONE
        DPOP    r1                      ;r1=n1
        cmp     r0,r1
        moveq   r0,1
        movne   r0,0
        NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'<> // (n1 n2 -- flag)  \ True if n1 <> n2",cmp_ne,T_NONE
        DPOP    r1                      ;r1=n1
        cmp     r0,r1
        moveq   r0,0
        movne   r0,1
        NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'< // (n1 n2 -- flag)  \ True if n1 < n2",cmp_lt,T_NONE
        DPOP    r1                      ;r1=n1
        cmp     r1,r0
        movge   r0,0                    ;<              
        movlt   r0,1
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'> // (n1 n2 -- flag)  \ True if n1 > n2",cmp_gt,T_NONE
        DPOP    r1                      ;r1=n1
        cmp     r0,r1
        movge   r0,0                    ;> // (r0 and r1 reversed)            
        movlt   r0,1
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'<= // (n1 n2 -- flag)  \ True if n1 <= n2",cmp_le,T_NONE
        DPOP    r1                      ;r1=n1
        cmp     r0,r1
        movge   r0,1                    ;<              
        movlt   r0,0
        NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'>= // (n1 n2 -- flag)  \ True if n1 > n2",cmp_ge,T_NONE
        DPOP    r1                      ;r1=n1
        cmp     r1,r0
        movge   r0,1                    ;<              
        movlt   r0,0
        NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'0= // (n1 -- flag)  \ True if n1 is 0",cmp_zr,T_NONE
        cmp     r0,0
        moveq   r0,1
        movne   r0,0
        NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'0<> // (n1 -- flag)  \ True if n1 is not 0",cmp_nz,T_NONE
        cmp     r0,0
        moveq   r0,1
        movne   r0,0
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'0< // (n1 -- flag)  \ True if n1 is less than 0",cmp_ltz,T_NONE
        cmp     r0,0
        movge   r0,1
        movlt   r0,0
        NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'0> // (n1 -- flag)  \ True if n1 is greater than 0",cmp_gtz,T_NONE
        cmp     r0,0
        movgt   r0,1
        movle   r0,0
        NEXT
.x:
;==============================================================================
; FORTH logical 
;------------------------------------------------------------------------------
CODE "system'core'and // (n1 n2 -- n1&n2)  \ logical and",log_and,T_NONE
        DPOP    r1              ;r1=n1
        and     r0,r1
        NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'or // (n1 n2 -- n1|n2)  \ logical or",log_or,T_NONE
        DPOP    r1              ;r1=n1
        orr     r0,r1
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'xor // (n1 n2 -- n1^n2)  \ logical xor",log_xor,T_NONE
        DPOP    r1              ;r1=n1
        eor     r0,r1
        NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'invert // (n1 -- ~n2)  \ bitwise not",bit_not,T_NONE
        ;RSB     r0,0
        mvn     r0,r0
        NEXT
.x: 

;==============================================================================
; FORTH shifts 
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
CODE "system'core'<< // (n1 n2 -- n1<<n2)  \ shift n1 left by n2 bits",lshift,T_NONE
        DPOP    r1              ;r1=n1
        lsl     r0,r1,r0
        NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'>> // (n1 n2 -- n1>>n2)  \ shift n1 right by n2 bits",rshift,T_NONE
        DPOP    r1              ;r1=n1
        lsr     r0,r1,r0
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'ror // (n1 n2 -- n)  \ rotate n1 right by n2 bits",rotr,T_NONE
        DPOP    r1              ;r1=n1
        ror     r0,r1,r0
        NEXT

.x:
;==============================================================================
; FORTH memory 
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
CODE "system'core'@ // (addr -- val)  \ fetch val from addr",fetch,T_NONE
        ldr     r0,[r0]
        NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'! // (val addr --)  \ store val at addr",store,T_NONE
        DPOP    r1              ;val
        str     r1,[r0]
        DPOP    r0
        NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'c@ // (addr -- val)  \ fetch val from addr",cfetch,T_NONE
        ldrb    r0,[r0]
        NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'c! // (val addr --)  \ store val at addr",cstore,T_NONE
        DPOP    r1              ;val
        strb    r1,[r0]
        DPOP    r0
        NEXT
.x: 
;------------------------------------------------------------------------------
CODE "system'core'c@++ // (addr -- addr+1 val) fetch and increment pointer",finc1,T_NONE
        mov     r1,r0                   ;addr
        ldrb    r0,[r1],1               ;val
        DPUSH   r1
        NEXT
.x:

;------------------------------------------------------------------------------
CODE "system'core'D- // (ah,al,bh,bl--ch,cl)",2sub,T_NONE
        ldr     r1,[DSP]        ;r1=bh
        ldr     r2,[DSP,4]      ;r2=al
        ldr     r3,[DSP,8]      ;r3=ah
        subs    r0,r2,r0        ;low
        sbc     r3,r3,r1
        str     r3,[DSP,8]
        add     DSP,8
        NEXT
.x:




;------------------------------------------------------------------------------
;
CODE "system'core'swap2 // (a,b,c,d--c,d,a,b)",swap2,T_NONE
        ldr     r1,[DSP]        ;r1=c
        ldr     r2,[DSP,4]      ;r2=b
        ldr     r3,[DSP,8];     ;r3=a
        str     r1,[DSP,8]
        str     r0,[DSP,4]
        str     r3,[DSP]
        mov     r0,r2
        NEXT
.x:

;------------------------------------------------------------------------------
CODE "system'core'push // (n--) push n onto ReturnStack",push,T_NONE
        RPUSH   r0
        DPOP    r0
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'core'pop // (--n) pop from return stack",pop,T_NONE
        DPUSH   r0
        RPOP    r0
        NEXT
.x:
   

;==============================================================================
;------------------------------------------------------------------------------
; U8 // (--U8)   load a U8 from codestream.
;
CODE "system'core'U8 // (--n) fetch a U8 that follows in the codestream",U8,T_U8
        DPUSH   r0
        ldrb    r0,[IP],1               ;fetch literal from [IP], increment
        NEXT
.x:
;------------------------------------------------------------------------------
; U16 // (--U16)  load a U16 from codestream.
;
CODE "system'core'U16 // (--n) fetch a U16 that follows in the codestream",U16,T_U16
        DPUSH   r0
        ldrh    r0,[IP],2               ;fetch literal from [IP], increment
        NEXT
.x:
;------------------------------------------------------------------------------
; U32 // (--U32)   load a 32 from codestream.
;
CODE "system'core'U32 // (--n) fetch a U32 that follows in the codestream",U32,T_U32
        DPUSH   r0
        ldr     r0,[IP],4               ;fetch literal from [IP], increment
        NEXT
.x:
;------------------------------------------------------------------------------
; REF // (--REF)   load a reference via table. ***TABLE
;
CODE "system'core'REF // (--n) fetch a REF that follows in the codestream",REF,T_REF
        ldrb    r1,[IP],1               ;r1 is tok used to fetch reference***
        lsr     r2,IP,4                 ;r2 is base
        lsls    r1,2                    ;r1 is tok*4, table offset
        DPUSH   r0
        ldr     r0,[r1,r2,LSL 2]        ;just like the interpreter
        NEXT
.x:
;------------------------------------------------------------------------------
; STRING  
;
CODE "system'core'STR8 // (--str,cnt) fetch a string pointer.  String follows inline",STR8,T_STR8

    DPUSH       r0              ;preserve TOS
    ldrb        r0,[IP],1       ;count
    DPUSH       IP              ;STR
    add         IP,r0
    NEXT
.x:
;------------------------------------------------------------------------------
; branch
;
CODE "system'core'branch // (--) branch by signed U8 offset",branchU8,T_OFF
        ldrsb   r1,[IP],1               ;get offset
        add     IP,r1
        NEXT
.x:
;------------------------------------------------------------------------------
;condition-code 0BRANCH OFFSET true-part rest-code
CODE "system'core'0branch // (cond--) if 0, branch by signed U8 offset",zbranchU8,T_OFF
        ldrsb   r1,[IP],1               ;get offset
        cmp     r0,0                    ;is TOS 0?
        addeq   IP,r1                   ;if not, add the offset...
        DPOP    r0                      ;eat the condition byte
        NEXT
.x:

;==============================================================================
;==============================================================================
;------------------------------------------------------------------------------
; >? // (a,b--a,?)  
CODE "system'core'>_",gt_,T_OFF
        ldr     r1,[DSP]        ;r1= a; keep a on dstack
        cmp     r1,r0
        movls   r0,1
        movgt   r0,0
        NEXT
.x:
;==============================================================================
; BRANCH
;------------------------------------------------------------------------------
; else # // (--)   unconditional jump to offset
;
CODE "system'core'else",else,T_OFF
        ldrsb   r1,[IP],1       ;load offset, increment IP
        add     IP,r1
        NEXT
.x:

;------------------------------------------------------------------------------
; times
;
; count on return stack.  Loop to offset. Clean up RSP at the end...
CODE "system'core'times // (cnt--) execute expression that follows cnt times",times,T_OFF
        ldr       r2,[RSP]              ;r2 is count
        ldrsb     r1,[IP],1             ;r1 is offset, IP++
        subs      r2,1                  ;decrement count
        addne     IP,r1                 ;if nz, loop
        strne     r2,[RSP]              ;if nz, update count on RSP
        bxne      lr 
        add       RSP,4                 ;if
        NEXT
.x:   
    
;==============================================================================
; variable
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
CODE "system'TYPE'U32'fetch // (--val)",var_fetchp,T_NONE
        DPUSH     r0
        ldrsb     r1,[IP],1             ;r1 is offset, IP++
        lsr       r2,IP,4               ;r2 is base ***
        lsls      r1,2                  ;r1 is tok*4, table offset
        ldr       r0,[r1,r2,LSL 2]      ;r0 is table entry
        ldr       r0,[r0]               ;value
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "system'TYPE'U32'into // (val--)",var_storep,T_NONE
        ldrsb     r1,[IP],1             ;r1 is offset, IP++
        lsr       r2,IP,4               ;r2 is base ***
        lsls      r1,2                  ;r1 is tok*4, table offset
        ldr       r1,[r1,r2,LSL 2]      ;r0 is table entry
        str       r0,[r1]
        DPOP      r0
        NEXT
.x:


;------------------------------------------------------------------------------

db 0    ;an empty record to terminate load process