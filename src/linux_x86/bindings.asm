;X86
;
format elf

section '.text' executable

;------------------------------------------------------------------------------
; invoke  the meow-meow interpreter
;
; (tablebase)
public meow_invoke
meow_invoke:
    mov         eax,[esp+4]             ;get tablebase off the stack
    pusha                       ;overkill
    
    ;mov         esp,[eax]       ;meow stack
    ;...
    ;mov         esp,
    popa
    ;mov         dword[eax],$DEADFEED
    mov         eax,[esp+4]             ;get tablebase off the stack
    mov         eax,[eax+44]            ;eax= sRegsMM structure
    mov         dword[eax],$1234
    
    ret
.x:

;=================================================================================================
; Inner interpreter.
; esi = IP
; eax = interpreter temp
;
public inner_interpreter
inner_interpreter:
    xor         eax,eax                 ;clear for byte token
    lodsb                               ;eax is token,
    shl         eax,2                   ;eax is index (token*4)
    jnz         .continue               ;zero is special
    call        esi                     ;call native code
    jmp         .loop
.continue:                              ;non-native interpreter
ret
