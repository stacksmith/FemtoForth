/******************************************************************************
Copyright 2014 Victor Yurkovsky

This file is part of the FemtoForth project.

FPGAsm is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

FemtoForth is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with FemtoForth. If not, see <http://www.gnu.org/licenses/>.

*****************************************************************************/
#include <string.h>
#include "global.h"
#include "header.h"
#include "src.h"
#include "interpret.h"
#include "cmd.h"
#include "table.h"
#include "lang.h"
#include "data.h"

// interpret.c

#include "cmd.h"
extern HINDEX search_list[];

 sVar* var;

/* ==========================================================
  Initialize the register contexts...
*/
extern U32 inner_interpreter;
char irpstr1[]="system';";
char irpstr2[]="system'leave";
HINDEX hreturn;
HINDEX hleave;
HINDEX hU8;
HINDEX hU16;
HINDEX hU32;
void interpret_init(){
    //Upon entry, the processor context must be on the stack...
    sRegsMM* p = var->sp_meow-1;       
//printf("interpret_init: sp_meow at %08x \n",p);
    p->TOS  = 0x9ABC;
    p->IP  = 0;             //IP will be set for the call
    p->DSP  = (U32*)var->dsp_top;  //DSP
    p->ER  = 0;             //exception register
    p->DAT = (U32)var;      //
    p->lr  = (U32)&inner_interpreter; //defined in bindings
    
//printf("inner interpreter is at %p\n",&inner_interpreter);
    var->sp_meow = p;
    // On another note, initialize some useful tokens
    //TODO: error-check that these are found!
    hleave = head_find_abs_or_die("core'leave");
    hU8  = head_find_abs_or_die("core'U8");
    hU16 = head_find_abs_or_die("core'U16");
    hU32 = head_find_abs_or_die("core'U32");
    
}
/* ============================================================================
 * comp  Compile a token representing header index.
 * 
 * This is hard.  Compile a token.  Its address is transformed into table base
 * which is searched for the target. If target is found, a token is calculated
 * from that table index.  If not, we attempt to insert a new target in this 
 * range.
 */

// U32 __attribute__((cdecl)) meow_invoke(U32);

void call_meow(U8* addr){
//printf("call_meow will run: %08X\n",addr);
    sRegsMM* pregs = (sRegsMM*)var->sp_meow;
   pregs->IP = (U32)addr;
  U32 ret=    meow_invoke(var); //var is in data!
//printf("call_meow: %08X\n",ret);

}



int interpret_literal_num(char* ptr,U32 cnt,U32 radix){
    char* endptr;
    U32 val = (U32)strtol(ptr,&endptr,radix);
    if(endptr != ptr+cnt){
//        printf("ptr %x cnt %x endptr %x\n",ptr,cnt,endptr);
        return 0;
    }
//printf("interpret_literal %d %x\n",val,val);
    //now compile U8,U16 or U32
    if(!(val&0xFFFFFF00)){
        data_compile_token(hU8);
        data_compile_U8(val);
    } else if(!(val&0xFFFF0000)){
        data_compile_token(hU16);
        data_compile_U16(val);
    } else {
        data_compile_token(hU32);
        data_compile_U32(val);
    }
    
    return 1;
}
/*=============================================================================
 * 'c' character literal
 * Note that a ' ' literal has the wrong count!
 * ==========================================================================*/
int interpret_literal_c(char* ptr,U32 cnt){
//printf("interpret_literal_c [%s] %x %d\n",ptr,ptr,cnt);
    data_compile_token(hU8);
    data_compile_U8(*(ptr+1));  //literal value...
    //handle the ' ' problem
    if(1==cnt) 
        var->src_ptr+=2;
    return 1;
}
int interpret_literal_str(char*ptr,U32 cnt){
    var->src_ptr = ptr; //cnt is not valid, back up truck
//printf("interpret_literal_str [%s] %d\n",ptr,cnt);
    data_compile_token(head_find_abs_or_die("core'STR8"));
    //TODO: add escape sequences
    U8* pcnt = data_compile_U8(0); //will fixup later
    cnt = 0;
    ptr++; //past the initial "
    while(1){
        char c = *ptr++;
        if('"' == c) break;
        cnt++;
        data_compile_U8(c);
    }
    var->src_ptr = ptr;
    if(cnt>255) return 0;
    *pcnt = cnt;
    return 1;
}
int interpret_literal(char* ptr,U32 cnt){
//printf("interpret_literal [%s] %x %d\n",ptr,ptr,cnt);
    U32 radix = 10;
    char first = *ptr;
    switch(first){
        case '$': return interpret_literal_num(ptr+1,cnt-1,16);
        case '\'': return interpret_literal_c(ptr,cnt);
        case '"': return interpret_literal_str(ptr,cnt);
        default:  return interpret_literal_num(ptr,cnt,10);
    }
    return 0;
}
extern int lang(char*ptr,U32 cnt);
/*
 * compile a single unit - lang, native or literal...
 */
extern HINDEX H_PROC;
extern HINDEX H_U32;
extern HINDEX H_TYPE;
//TODO refprim...

/* ==========================================================
  handle source attachment
*/
//defining
int interpret_def_source(HINDEX h){
    src_ws(); //
    head_append_source(h,var->src_ptr,0); //append this first line
    //now read lines and append as source until a line containing only "end"
    while(1){
        char*psrc = src_line();         //a fresh line of source
        U32 len = strlen(psrc);        
//printf("lang_colon [%s] %d\n",psrc,len);
        //terminate source with a line containing an |
        if((2==len)&&('|'==*psrc)){
            var->src_ptr += len;
            break;
        }
        head_append_source(h,psrc,len);
    }
    head_commit(h);
//printf("interpret_def_source done...[%s]\n",head_get_source(h));

}
int interpret_def_PROC(HINDEX h){
    src_set(head_get_source(h));
}
int interpret_def_U32(HINDEX h){
    var->data_ptr +=4;
//printf("interpret_def_U32 [%s]\n",var->src_ptr);
}

int interpret_def_type(HINDEX htype){
//printf("interpret_def_type: type [%.*s]\n",head_get_namelen(htype),head_get_name(htype));
    //TODO: check for duplication...of string and of datatptr...
    HINDEX h = head_new(var->data_ptr,  htype,T_NA, search_list[0]);

    interpret_def_source(h); //in any event, append source and commit entry
//printf("interpret_def: src [%s]\n",var->src_ptr);
//printf("lang_colon: type %.*s %d \n",head_get_namelen(htype),head_get_name(htype));
    //dispatch on type! Note: tcnt and tname are not valid, as source has changed
    int ret=0;
    if(htype == H_PROC)
        ret = interpret_def_PROC(h);
    else if(htype == H_U32)
        ret = interpret_def_U32(h);
    //if compile OK, commit the data
    if(ret)
        var->run_ptr = var->data_ptr;
    return ret;
    
}

int interpret_compone(char* ptr,U32 cnt){
//printf("interpret_compone[%s] %d\n",ptr,cnt);
    //--------------------------------------------------------------
    // 'x' char literals. Note: ' ' breaks the pattern as the count
    // is 1 due to the space following the initial quote.  Luckily,
    // a ' ' sequence must always represent a literal...
    if(('\''==*ptr)&&('\'')==*ptr+2) 
        return  interpret_literal_c(ptr,cnt);
    //--------------------------------------------------------------
    // language constructs (compiling words)
    if(lang(ptr,cnt)) return 1;
    //--------------------------------------------------------------
    // regular words
    HINDEX h = head_find(ptr, cnt,search_list);
    if(h) {
        //simple type dispatch.  Real language dispatches better..
        HINDEX type = head_get_type(h);
        if(type==H_TYPE)
            return interpret_def_type(h);
        if(type==H_PROC)
            return data_compile_token(h);                  //compile a token...
        if(type==H_U32){
            return data_ref_style(h,"TYPE'U32'prim'compile");
        }
    }
    //--------------------------------------------------------------
    // numeric literal
    return interpret_literal(ptr,cnt);        //finally try literal
}

int interpret_one(){
    U32 cnt; char* ptr = src_word(&cnt);
    return interpret_compone(ptr,cnt);
}


int interpret_compuntil(char* delim, U32 delimcnt){
    while(1){
        U32 cnt; char* ptr = src_word(&cnt);
        if((delimcnt==cnt)&&(0==strncmp(delim,ptr,cnt)))
            return 1;
        if(!interpret_compone(ptr,cnt)) {
            printf("ERROR [[%s]]\n",var->src_ptr);
            return 0;
        }
         
    }
    return 1;
}

int command(char* ptr,U32 cnt);


int interpret_outer_p(char* ptr,U32 cnt){
    if(!command(ptr,cnt)) 
        if(!interpret_compone(ptr,cnt)) {        //otherwise, do the magic
            src_error("not found:");
            *var->src_ptr=0;                         //abandon line!
            return 0;
         }
    return 1;
}
int interpret_outer(){
//printf("interpret_outer\n");
    U32 cnt;
    char* ptr = src_word(&cnt);
    //---------------------------------------
    //Prepare to clean up our tracks...
    var->run_ptr = var->data_ptr;
    var->run_table = table_end(var->data_ptr);
//printf("in: run_table is at %08p\n",var->run_table);
    interpret_outer_p(ptr,cnt);
    //execute
//    if(var->run_ptr != var->data_ptr) {
//        data_compile_U8(0);
        data_compile_token(hleave);                    //terminate with a return
//printf("--%p\n",var->data_ptr);
//lang_ql(var->run_ptr);//
        call_meow(var->run_ptr);                    //run from run_ptr
//    }      
    //---------------------------------------
    // reset after run
    memset(var->run_ptr,0xFF,(var->data_ptr - var->run_ptr));
//printf("out: run_table is at %08p\n",var->run_table);
    var->data_ptr = var->run_ptr;               //and reset
    table_wipe(var->run_table);
    return 1;
   
}

