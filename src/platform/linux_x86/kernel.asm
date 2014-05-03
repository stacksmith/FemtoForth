; kernel.asm
;
; used do build the kernel for meow-meow.
;
; format: described in macro below
;
use32

include "../kernel_common.s"
; Register usage:
DSP equ ebp
TOS equ eax


macro NEXT {
    jmp         edi
}
macro DPUSH reg{
    sub         ebp,4           ;DPUSH
    mov         dword[ebp],reg  ;
}
macro DPOP reg {
    mov         reg,[ebp]
    add         ebp,4
}
; Format:
; 1 cnt   count of string, including null-term and padding
; ? name
; 1 parm  - tokenstream data 
;
; The loader will prefix each code word with a 0, properly aligned!
macro CODE str,name,parm {
  db .z2-.z1
.z1: db str,0
align 4
.z2:
; 
db parm
db 0,0,0
dd __#name#.x - __#name         ;DANGER: x86 d is 4 bytes,w is 2!
__#name:
    ;RPOP IP
}
; return to C
CODE "core'leave // exit to outer host ",leave,T_NONE 
 ;and in reverse.. interpreter is already on the stack!
    mov         ecx,DWORD $04000000;    ;TODO:DATA!*** THIS SUCKS
    
    push        edi                     ;vm pointer
    push        ecx                     ;dat TODO:*** THIS SUCKS
    push        DWORD 0                 ;er
    push        ebp                     ;DSP
    push        esi                     ;IP
    push        eax                     ;TOS
    mov         DWORD[ecx+SP_MEOW],esp  ;save meow stack pointer

    mov         esp,[ecx+SP_C]          ;restore c stack
    pop         edi
    pop         esi
    pop         ebp
    pop         ecx
    pop         ebx

    ret

.x:
;------------------------------------------------------------------------------
CODE "io'putc // (c,handle--)",putc1,T_NONE                      ;(c --)
    pusha
    mov         eax,4                   ;fwrite
    mov         ebx,[esp+28]            ;handle in TOS
    mov         ecx,ebp                 ;buffer from NOS
    mov         edx,1
    int         0x80
    popa    
    mov         eax,[ebp+4]
    add         ebp,8
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "io'getc // (handle--c)",getc1,T_NONE                      ;(c --)
    DPUSH       eax
    pusha
    mov         eax,3                   ;fread
    mov         ebx,[esp+28]            ;handle in TOS
    lea         ecx,[esp+28]
    mov         edx,1
    int         0x80
    popa   
    and         eax,$FF
    NEXT
.x:

;------------------------------------------------------------------------------
CODE "sys'time",sys_time,T_NONE
    DPUSH       eax
    pusha                               ;eax,ecx,edx,ebx,?,ebp,esi,edi
    mov         eax,0x0D                ;sys_time
    lea         ebx,[esp+28]            ;eax will return value
    int         0x80
    popa
    
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "sys'gettimeofday // (--Sec,uSec)",sys_gettimeofday,T_NONE
    DPUSH       eax
    pusha
    mov         eax,78                  ;sys_timeofday
    lea         ebx,[esp+24]            ;eax
    xor         ecx,ecx                 ;no tz
    int         0x80
    popa
    DPUSH       ecx
    NEXT
.x:

;==============================================================================
; FORTH basics
;------------------------------------------------------------------------------
; else # (--)   unconditional jump to offset
;
CODE "core'DSP // (--DSP) get DataStack pointer",DSP,T_NONE
        DPUSH   eax
        mov     eax,DSP
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "core'dup // (n -- n n) Duplicates the top stack item.",dup,T_NONE
        DPUSH   eax
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "core'drop // (n --) Discards the top stack item.",drop,T_NONE
        DPOP    eax
        NEXT
.x:

;------------------------------------------------------------------------------
;
CODE "core'swap // (n1 n2 -- n2 n1) Reverses the top two stack items.",swap,T_NONE
        xchg    eax,[ebp]
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "core'over // (n1 n2 -- n1 n2 n1) Makes a copy of the second item and pushes it on top.",over,T_NONE
        DPUSH   eax
        mov     eax,[ebp+4]
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "core'rot // (a b c -- b c a) Rotates the third item to the top.",rot,T_NONE
        DPOP    ebx             ;ebx=n2
        DPOP    ecx             ;ecx=n1
        DPUSH   ebx
        DPUSH   eax
        mov     eax,ecx
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "core'-rot // (a b c -- c a b) rotate the first item to third.",minusrot,T_NONE
        DPOP    ebx             ;ebx=b
        DPOP    ecx             ;ecx=a
        DPUSH   eax
        DPUSH   ecx
        mov     eax,ebx
        NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "core'?dup // (a -- a a | 0) duplicate top of stack if non-zero",conddup,T_NONE
        test     eax,eax
        jz      .done
        DPUSH   eax
.done:  NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "core'1+ // (a -- a+1) increment",incr,T_NONE
        inc     eax
.done:  NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "core'1- // (a -- a-1) decrement",decr,T_NONE
        dec     eax
.done:  NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "core'4+ // (a -- a+4) increment by 4",incr4,T_NONE
        add     eax,4
.done:  NEXT
.x:
;------------------------------------------------------------------------------
;
CODE "core'4- // (a -- a-14) decrement by 4",decr4,T_NONE
        sub     eax,4
.done:  NEXT
.x:

;------------------------------------------------------------------------------
CODE "core'+ // (a,b--sum)",add,T_NONE
    add         eax,[ebp]
    add         ebp,4
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "core'- // (a,b--(a-b))",sub,T_NONE
    mov         ebx,[ebp]       ;ebx = a
    sub         ebx,eax
    add         ebp,4
    mov         eax,ebx
    NEXT
.x:
;------------------------------------------------------------------------------
CODE "core'* // (a,b--a*b)",mul,T_NONE
    DPOP        ebx
    imul        ebx
    NEXT
.x:
;==============================================================================
; FORTH comparisons
;------------------------------------------------------------------------------
CODE "core'= // (n1 n2 -- flag)   True if n1 = n2",cmp_eq,T_NONE
    xor         ebx,ebx
    cmp         eax,[ebp]
    setz        bl
    add         ebp,4
    mov         eax,ebx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'<> // (n1 n2 -- flag)  \ True if n1 <> n2",cmp_ne,T_NONE
    xor         ebx,ebx
    cmp         eax,[ebp]
    setne        bl
    add         ebp,4
    mov         eax,ebx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'< // (n1 n2 -- flag)  \ True if n1 < n2",cmp_lt,T_NONE
    xor         ebx,ebx
    cmp         [ebp],eax
    setl        bl
    add         ebp,4
    mov         eax,ebx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'> // (n1 n2 -- flag)  \ True if n1 > n2",cmp_gt,T_NONE
    xor         ebx,ebx
    cmp         [ebp],eax
    setg       bl
    add         ebp,4
    mov         eax,ebx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'<= // (n1 n2 -- flag)  \ True if n1 <= n2",cmp_le,T_NONE
    xor         ebx,ebx
    cmp         [ebp],eax
    setle       bl
    add         ebp,4
    mov         eax,ebx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'>= // (n1 n2 -- flag)  \ True if n1 > n2",cmp_ge,T_NONE
    xor         ebx,ebx
    cmp         [ebp],eax
    setge       bl
    add         ebp,4
    mov         eax,ebx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'0= // (n1 -- flag)  \ True if n1 is 0",cmp_zr,T_NONE
    xor         ebx,ebx
    cmp         ebx,eax
    sete        bl
    mov         eax,ebx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'0<> // (n1 -- flag)  \ True if n1 is not 0",cmp_nz,T_NONE
    xor         ebx,ebx
    cmp         ebx,eax
    setne       bl
    mov         eax,ebx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'0< // (n1 -- flag)  \ True if n1 is less than 0",cmp_ltz,T_NONE
    xor         ebx,ebx
    test        eax,eax
    setl        bl
    mov         eax,ebx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'0> // (n1 -- flag)  \ True if n1 is greater than 0",cmp_gtz,T_NONE
    xor         ebx,ebx
    test        eax,eax
    setg        bl
    mov         eax,ebx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'0<= // (n1 -- flag)  \ True if n1 is less then or equal to 0",cmp_lez,T_NONE
    xor         ebx,ebx
    test        eax,eax
    setle        bl
    mov         eax,ebx
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'0>= // (n1 -- flag)  \ True if n1 is greater then or equal to 0",cmp_gez,T_NONE
    xor         ebx,ebx
    test        eax,eax
    setge        bl
    mov         eax,ebx
    NEXT
.x: 
;==============================================================================
;==============================================================================
; special conditionals.  RHS is destroyed, but not lhs!
CODE "core'if'> // (n1 n2 -- n1) conditionally execute the following expression",if_gt,T_NONE
    xor         ecx,ecx                 ;offset
    cmp         [ebp],eax               ;compare n1,n2
     setle       cl                      ;if n1 <= n2
    movsx       ebx,byte[esi]           ;ebx is offset
     sub         ecx,1                   ;if n1 <= n2, 0.  if n1>n2, 0xFFFF
    add         esi,1
     and         ebx,ecx
 ;   add         esi,ebx                 ;add offset or 0
    DPOP        eax
    NEXT
.x:
;==============================================================================
; FORTH logical 
;------------------------------------------------------------------------------
CODE "core'and // (n1 n2 -- n1&n2)  \ logical and",log_and,T_NONE
    and         eax,[ebp]
    add         ebp,4
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'or // (n1 n2 -- n1|n2)  \ logical or",log_or,T_NONE
    or         eax,[ebp]
    add         ebp,4
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'xor // (n1 n2 -- n1^n2)  \ logical xor",log_xor,T_NONE
    xor         eax,[ebp]
    add         ebp,4
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'invert // (n1 -- ~n1)  \ bitwise not",bit_not,T_NONE
    not         eax
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'negate // (n1 -- 0-n1)  \ arithmetic negation",negate,T_NONE
    neg         eax
    NEXT
.x: 

;==============================================================================
; FORTH shifts 
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
CODE "core'<< // (n1 n2 -- n1<<n2)  \ shift n1 left by n2 bits",lshift,T_NONE
    mov         ecx,eax
    DPOP        eax
    shl         eax,cl
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'>> // (n1 n2 -- n1>>n2)  \ shift n1 right by n2 bits",rshift,T_NONE
    mov         ecx,eax
    DPOP        eax
    shr         eax,cl
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'ror // (n1 n2 -- n)  \ rotate n1 right by n2 bits",rotr,T_NONE
    mov         ecx,eax
    DPOP        eax
    ror         eax,cl
    NEXT
.x: 
;==============================================================================
; FORTH memory 
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
CODE "core'@ // (addr -- val)  \ fetch val from addr",fetch,T_NONE
    mov         eax,[eax]
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'! // (val addr --)  \ store val at addr",store,T_NONE
    mov         ebx,[ebp]
    add         ebp,4
    mov         [eax],ebx
    DPOP        eax
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'c@ // (addr -- val)  \ fetch val from addr",cfetch,T_NONE
    mov         al,[eax]
    NEXT
.x: 
;------------------------------------------------------------------------------
CODE "core'c! // (val addr --)  \ store val at addr",cstore,T_NONE
    mov         ebx,[ebp]
    add         ebp,4
    mov         [eax],bl
    DPOP        eax
    NEXT
.x: 

;------------------------------------------------------------------------------
CODE "core'c@++ // (addr -- addr+1 val) fetch and increment pointer",finc1,T_NONE
        mov     ebx,eax
        xor     eax,eax
        mov     al,[ebx]
        add     ebx,1
        DPUSH   ebx
        NEXT
.x:
        
;------------------------------------------------------------------------------
;
CODE "core'swap2 // (a,b,c,d--c,d,a,b)",swap2,T_NONE
        xchg    eax,[ebp+4]     ;a,d,c,b
        mov     ebx,[ebp]
        xchg    ebx,[ebp+8]     ;c,d,a,b
        mov     [ebp],ebx
        NEXT
.x:

;------------------------------------------------------------------------------
CODE "core'push // (n--) push n onto ReturnStack",push,T_NONE
        push    eax
        DPOP    eax
        NEXT
.x:
;------------------------------------------------------------------------------
CODE "core'pop // (--n) pop from return stack",pop,T_NONE
        DPUSH   eax
        pop     eax
        NEXT
.x:

;==============================================================================
; Literals (from codestream)
;------------------------------------------------------------------------------
; U8 (--U8)   load a U8 from codestream.
;
CODE "core'U8 // (--n) fetch a U8 that follows in the codestream",U8,T_U8
    DPUSH       eax
    xor         eax,eax
    mov         al,[esi]
    add         esi,1
    NEXT
.x:
;------------------------------------------------------------------------------
; U16 (--U16)  load a U16 from codestream.
;
CODE "core'U16 // (--n) fetch a U16 that follows in the codestream",U16,T_U16
    DPUSH       eax
    xor         eax,eax
    mov         ax,[esi]
    add         esi,2
    NEXT
.x:
;------------------------------------------------------------------------------
; U32 (--U32)   load a 32 from codestream.
;
CODE "core'U32 // (--n) fetch a U32 that follows in the codestream",U32,T_U32
    DPUSH       eax
    xor         eax,eax
    mov         eax,[esi]
    add         esi,4
    NEXT
.x:
;------------------------------------------------------------------------------
; REF (--REF)   load a reference via table. ***TABLE
;
CODE "core'REF // (--n) fetch a REF that follows in the codestream",REF,T_REF
    DPUSH       eax             ;just like U32 fetch
    xor         eax,eax
    mov         al,[esi]
    add         esi,1
    mov         ecx,esi         ;calculate table base
    shr         ecx,4
    shl         ecx,2
    ;
    mov         eax,[ecx+eax*4]
    NEXT
.x:
;------------------------------------------------------------------------------
; STRING  
;
CODE "core'STR8 // (--str,cnt) fetch a string pointer.  String follows inline",STR8,T_STR8
    DPUSH       eax             ;preserve tos
     xor         eax,eax        ;load count into TOS
     mov         al,[esi]
     add         esi,1
    DPUSH       esi             ;STR
    add         esi,eax         ;skip string
    NEXT
 .x:   


;------------------------------------------------------------------------------
; branch
;
CODE "core'branch // (--) branch by signed U8 offset",branchU8,T_OFF
    movsx       ebx,byte[esi]
    add         esi,1
    add         esi,ebx
    NEXT
.x:    
;condition-code 0BRANCH OFFSET true-part rest-code
CODE "core'0branch // (cond--) if 0, branch by signed U8 offset",zbranchU8,T_OFF
    movsx       ebx,byte[esi]           ;ebx is offset
    add         esi,1
    and         eax,1
    dec         eax                     ;0->FFFFFFFF, 1->0
    and         ebx,eax
    add         esi,ebx                 ;add offset or 0
    DPOP        eax
    NEXT
.x:
;------------------------------------------------------------------------------
; times
;
; count on return stack.  Loop to offset. Clean up RSP at the end...
CODE "core'times // (--) execute expression that follows cnt times",times,T_OFF
        sub         DWORD[esp],1
        jz          .z
        movsx       ebx,byte[esi]
        add         esi,1
        add         esi,ebx
        NEXT
.z:     add         esi,1               ;skip offset
        add         esp,4               ;get rid of the 0 on rsp
        NEXT
.x:


;==============================================================================

;------------------------------------------------------------------------------
CODE "core'D- // (ah,al,bh,bl--ch,cl)",2sub,T_NONE
    mov         ebx,[ebp+4]       ;ebx = al
    sub         ebx,eax
    mov         eax,ebx           ;low done
    mov         ebx,[ebp+8]       ;ebx = ah
    sbb         ebx,[ebp]
    add         ebp,12    
    mov         [ebp],ebx
    NEXT
.x:


;------------------------------------------------------------------------------
CODE "test'nop ",nop,T_NONE
    sub         ebp,4
    mov         [ebp],eax
    mov         eax,$DEADDEAD
    NEXT
.x:
;==============================================================================
; variable
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
CODE "TYPE'U32'prim'compile // (--val)",var_fetchp,T_NONE
    DPUSH       eax          
     xor         eax,eax
     mov         al,[esi]        ;next token
     add         esi,1
    mov         ecx,esi         ;calculate table base ***
    shr         ecx,4
    shl         ecx,2
    ;
    mov         eax,[ecx+eax*4]
    mov         eax,[eax]
    NEXT
.x:
;==============================================================================
; store variable
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
CODE "TYPE'U32'prim'into // (val--)",var_storep,T_NONE
     xor         ebx,ebx
     mov         bl,[esi]        ;next token
     add         esi,1
    mov         ecx,esi         ;calculate table base ***
    shr         ecx,4
    shl         ecx,2
    ;
    mov         ebx,[ecx+ebx*4]         ;address of variable
    mov         [ebx],eax
    DPOP        eax
    NEXT
.x:
;------------------------------------------------------------------------------
db 0    ;an empty record to terminate load process