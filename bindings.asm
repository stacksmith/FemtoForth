format elf

section '.text' executable

IP equ r6
macro RPUSH reg {
  str reg,[RSP,-4]!
}
RDAT equ r11
SP_C       equ 44
SP_MEOW    equ 48

;------------------------------------------------------------------------------
; invoke  the meow-meow interpreter
;
; r0 = var
public meow_invoke
meow_invoke:

       push    {r4-r11,lr}              ;preserve C context on C stack...
       str     sp,[r0,SP_C]             ;save C sp..
       ldr     sp,[r0,SP_MEOW]
       pop     {r0,r6,r7,r9,r11,lr}     ;restore meow registers...
      
;       bx lr                            ;and jump into the interpreter
       
        push    {r0,r6,r7,r9,r11,lr}
        str     sp,[RDAT,SP_MEOW]                  ;consider not storing for reentrancy
        ldr     sp,[RDAT,SP_C]
        pop     {r4-r11,lr}
        bx      lr
.x:

;=================================================================================================
; Inner interpreter.
; r3 = tokenN /table index.
public inner_interpreter
inner_interpreter:

innerl: ldrb    r12,[IP],1               ;2; fetch a token
        lsls    r12,2                    ;1; r3 = table index; set Z if code.
        bxeq    IP                       ;2; 0=CODE! 
        str     IP,[sp,-4]!
        lsr     IP,IP,4                  ;1; create a 16-byte range address
        ldr     IP,[r12,IP,LSL 2]        ;2; r2 is return IP (dereference table)
        b       innerl                   ;1;

