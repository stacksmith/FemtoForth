  format elf
  section '.text' executable
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
macro CODE name {
  public name
  public name#_x
name:   
}
CODE init
        subs    DSP,sp,$200                     ;setup data stack
       
init_x:
CODE emit
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
emit_x:  
CODE key
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
key_x:


