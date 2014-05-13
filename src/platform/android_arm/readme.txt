ARM-32-linux notes

Register usage:

REG             VM              KERNEL          PRESERVE?
=========================================================
R0              -               TOS
R1              -                
R2              -               
R3             tok              scratch
R5              -               ERR
R6             IP                   
R7              -               DSP
lr             vmptr
sp             SP               SP
R11             -               DATA




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
