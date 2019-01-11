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
;X86
;
format elf
use32

section '.text' executable
include "../kernel_common.s"
;//SP_C       equ 0
;SP_MEOW    equ 4

;------------------------------------------------------------------------------
; invoke  the meow-meow interpreter
;
; (tablebase)
public meow_invoke
meow_invoke:

    mov         ebx,[esp+4]             ;get database off the stack
    ;preserve C registers
    push        ebx
    push        ecx
    push        ebp
    push        esi
    push        edi
    
    mov         DWORD[ebx+SP_C],esp     ;save c stack
    ;load meow registers from structure passed to this function.
    ;I am thinking of multithreading etc.
    mov         esp,[ebx+SP_MEOW]       ;load meow stack
    pop         eax                     ;TOS
    pop         esi                     ;IP
    pop         ebp                     ;DSP
    add         esp,8                   ;ER
    pop         edi                     ;EDI=interp
    ; invoke
    jmp         edi

;and in reverse.. interpreter is already on the stack!
    push        edi                     ;vm pointer
    push        DWORD $04000000         ;dat TODO:*** THIS SUCKS
    push        DWORD 0                 ;er
    push        ebp                     ;DSP
    push        esi                     ;IP
    push        eax                     ;TOS
    mov         DWORD[ecx+SP_MEOW],esp  ;save meow stack pointer

    mov         ecx,DWORD $04000000;    ;TODO:*** THIS SUCKS
    mov         esp,[ecx+SP_C]          ;restore c stack
    pop         edi
    pop         esi
    pop         ebp
    pop         ecx
    pop         ebx

    ret
.x:
;=================================================================================================
; Inner interpreter.
;
; trashes ecx,edx
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
return:
    pop       esi
inner_interpreter:
    movzx     edx,byte[esi]              ;al=tok, inc esi
    add	      esi,1
    shl       edx,2                 ;token*4 (pointers are 4-bytes), set flags...
    jz        return                ;A 0 token that is not first means return
.inner_loop:
    push      esi                   ;thread in...
    shr       esi,4                 ;Tricky: esi shr 4 then shl 2 for alignment
    mov       esi,[esi*4+edx]       ;and index it with token*4 resulting in CALL
    movzx     edx,byte[esi]              ;al=tok, inc esi
    add       esi,1
    shl       edx,2                 ;first byte of subroutine 0? Machine language code follows
    jnz       .inner_loop            ;continue threading
    mov       ecx,esi               ;routine address
    pop       esi                   ;no stacking of IP...
    jmp       ecx                   ;call assembly subroutine that follows


