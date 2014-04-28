;ARM7
;
format elf

section '.text' executable
RDAT equ r11
RSP equ sp

IP equ r6
macro RPUSH reg {
  str reg,[RSP,-4]!
}
macro RPOP reg {
  ldr reg,[RSP],4
}

;UPDATE THESE WHEN sVar changes!
SP_C       equ 40
SP_MEOW    equ 44

;------------------------------------------------------------------------------
; invoke  the meow-meow interpreter
;
; r0 = var
public meow_invoke
meow_invoke:

       push    {r4-r11,lr}              ;preserve C context on C stack...
       str     sp,[r0,SP_C]             ;save C sp in data area..
       ldr     sp,[r0,SP_MEOW]
       pop     {r0,r6,r7,r9,r11,lr}     ;restore meow registers...
       bx lr                            ;and jump into the interpreter

; 'system'leave will exit...
       
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

;innerl: ldrb    r12,[IP],1               ;2; fetch a token
;        lsls    r12,2                    ;1; r3 = table index; set Z if code.
;        bxeq    IP                       ;2; 0=CODE! 
;        str     IP,[sp,-4]!
;        lsr     IP,IP,4                  ;1; create a 16-byte range address
;        ldr     IP,[r12,IP,LSL 2]        ;2; r2 is return IP (dereference table)
;        b       innerl                   ;1;
; TODO: entry point?
innerl:
        ldrb     r3,[IP],1             ;2; load token
        lsls     r3,2                  ; ; r3 = table index (token*4)
        beq      inner3                  ;zero? return
inner_loop:
        lsr      r2,IP,4                 ;r2 is table base/4
        ldr      r2,[r3,r2,LSL 2]        ;r2 is new IP, but not yet...
        ldrb     r3,[r2],1               ;r3 = token from target
        lsls     r3,2                    ;r3 = table index from target
        bxeq     r2                      ;zero? CODE! execute it..
        RPUSH    IP                      ;not code!  push return address
        mov      IP,r2
        b        inner_loop
inner3:
        RPOP     IP
        b        innerl

