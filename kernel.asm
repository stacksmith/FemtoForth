; kernel.asm
;
; used do build the kernel for meow-meow.
;
; format: described in macro below
;

DATA_BASE  equ 0
DATA_TOP   equ 4
TABLE_BASE equ 8
TABLE_TOP  equ 12
DSP_BASE   equ 16
DSP_TOP    equ 20
RSP_BASE   equ 24
RSP_TOP    equ 28
HTABLE_PTR equ 32
HTABLE_TOP equ 36

DATA_PTR   equ 40
TABLE_PTR  equ 44
RUN_PTR    equ 48
SP_C       equ 52
SP_MEOW    equ 56
; Register usage:

IP equ r6
IP! equ r6!

DSP equ r7
DSP! equ r7!

ER equ r9
ER! equ r9!

RDAT equ r11            ;data segment register...

RSP equ sp
RSP! equ sp!

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
T_NONE  equ 0
T_U8    equ 1
T_U16   equ 2a
T_U32   equ 3
T_OFF   equ 4
T_STR   equ 5

TYPE_PROC equ 3
; Format:
; 1 cnt   count of string, including null-term and padding
; ? name
; 1 parm  - tokenstream data 

macro CODE str,name,parm {
  db .z2-.z1
.z1: db str,0
align 4
.z2:
; 
db parm
db 0,0,0
dw __#name#.x - __#name
__#name:
    RPOP IP
}
; return to C
CODE "system'leave",leave,T_NONE 
        push    {r0,r6,r7,r9,r11,lr}
        str     sp,[RDAT,SP_MEOW]                  ;consider not storing for reentrancy
        ldr     sp,[RDAT,SP_C]
        pop     {r4-r11,lr}
        bx      lr
.x:

CODE "test'a",temit,T_NONE 
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
        

CODE "system'irp1",irp1,T_NONE
.1: ldrb    r12,[IP],1               ;2; fetch a token
        lsls    r12,2                    ;1; r3 = table index; set Z if code.
        bxeq    IP                      ;2; 0=CODE! 
        RPUSH   IP
        lsr     IP,IP,4                 ;1; create a 16-byte range address
        ldr     IP,[r12,IP,LSL 2]        ;2; r2 is return IP (dereference table)
        b       .1                  ;1;
; Observations:
; lr must be set to innerl.  Subroutines must preserve AND RESTORE lr!
.x:

CODE "system';",return,T_NONE
    RPOP        IP
    bx          lr
.x:

;
CODE "io'emit",emit,T_NONE 
        push    {r0-r7,lr}
        mov     r0,1                            ;stdout
        add     r1,RSP,4                        ;char is RSP[4]
        mov     r2,1
        mov     r7,4                            ;write
        swi     0
        add     DSP,4
        pop     {r0-r7,lr}
        ldr     r1,[DSP],4                           ;drop
        bx      lr
.x:  

CODE "io'key",key,T_NONE
        DPUSH   r1                              ;preserve TOS
        push    {r0-r7,lr}
        mov     r0,0                            ;stdin
        add     r1,RSP,4                        ;char is RSP[4]
        mov     r2,1
        mov     r7,3                            ;read
        swi     0
        pop     {r0-r7,lr}                      ;TOS gets key buffer
        and     r1,$FF
        bx      lr
.x:


CODE "io'ttt",ttt,T_NONE
.x:

db 0