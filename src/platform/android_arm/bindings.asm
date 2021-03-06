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
 ;ARM7
;
format elf

section '.text' executable
include "../kernel_common.s"

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
;//SP_C       equ 0
;SP_MEOW    equ 4

;------------------------------------------------------------------------------
; invoke  the meow-meow interpreter
;
; r0 = var
public meow_invoke
meow_invoke:
       push    {r4-r11,lr}              ;preserve C context on C stack...
       mov     RDAT,r0
       str     sp,[RDAT,SP_C]             ;save C sp in data area..
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
; 
; r3 = token, *4=table index (for 32-bit tables)
; r2 = next IP, if not pointing to code will become IP
;
; A partially unrolled interpreter.  Upon entry into a subroutine, the first
; token is special: if it is zero, the subroutine is code.  Otherwise, token
; zero means return from subroutine.  Obviously,  empty subroutines are not
; allowed.
;
; Upon entry to code subroutines, IP points above, at the next token (not at
; code).
;
public inner_interpreter
;
nop
nop
nop
nop
nop
nop
nop
nop
; 
return:
        RPOP     IP                         ;restore IP from stack, returning
inner_interpreter:
        ldrb     r3,[IP],1             ;2; load token
        lsls     r3,2                  ; ; r3 = table index (token*4)
        beq      return                  ;first token zero? RETURN
inner_loop:
        lsr      r2,IP,4                 ;r2 is table base/4
        ldr      r2,[r3,r2,LSL 2]        ;r2 will become IP if not code...
        ldrb     r3,[r2],1               ;r3 = token from target
        lsls     r3,2                    ;r3 = table index from target
        bxeq     r2                      ;zero? CODE! execute it..
        RPUSH    IP                      ;not code!  push return address
        mov      IP,r2
        b        inner_loop

