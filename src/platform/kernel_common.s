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

; reserved space at the bottom of the data area.  See global.h
HOST_RESERVED equ 128
SYS_RESERVED equ 128

; in sVar structure located past HOST_RESERVED
SP_C       equ HOST_RESERVED+0
SP_MEOW    equ HOST_RESERVED+4

ERROR_FRAME equ 80
;DATA_BASE  equ 0
;DATA_TOP   equ 4
;TABLE_BASE equ 8
;TABLE_TOP  equ 12
;DSP_BASE   equ 16
;DSP_TOP    equ 20
;RSP_BASE   equ 24
;RSP_TOP    equ 28

;DATA_PTR   equ 32
;RUN_PTR  equ 36
;RUN_PTR    equ 40
; parsing parameter types
T_NONE  equ 0
T_U8    equ 1
T_U16   equ 2
T_U32   equ 3
T_OFF   equ 4
T_STR8   equ 5
T_REF   equ 6

TYPE_PROC equ 3

