; kernel.asm
;
; used do build the kernel for meow-meow.
;
; format:
; null-terminated fully-qualified name.  The path will be created
; U16 code size
; code...

IP equ r6
IP! equ r6!

DSP equ r7
DSP! equ r7!

ER equ r9
ER! equ r9!

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
T_U16   equ 2
T_U32   equ 3
T_OFF   equ 4
T_STR   equ 5

TYPE_PROC equ 3
; Format:
; 1 cnt   count of string, including null-term and padding
; ? name
; 1 parm  - tokenstream data 

macro CODE str,name,parm {
  db .2-.1
.1: db str,0
align 4
.2:
; 
db parm
db 0,0,0
dw name#.x - name
name:
}
      
CODE "io'emit",emit,T_NONE 
db $12,$34,$56,$78
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
      mov       r0,0x1234
      bx        lr

.x:

db 0