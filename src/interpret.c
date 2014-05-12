/******************************************************************************
Copyright 2014 Victor Yurkovsky

This file is part of the FemtoForth project.

FemtoForth is free software: you can redistribute it and/or modify
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
#include "color.h"

// interpret.c

#include "cmd.h"
extern HINDEX search_list[];

 sVar* var;
sMemLayout* lay;

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
    p->DSP  = (U32*)lay->dsp_top;  //DSP
    p->ER  = 0;             //exception register
    p->DAT = (U32)lay;      //
    p->lr  = (U32)&inner_interpreter; //defined in bindings
    
//printf("inner interpreter is at %p\n",&inner_interpreter);
    var->sp_meow = p;
    // On another note, initialize some useful tokens
    //TODO: error-check that these are found!
    hleave = head_find_abs_or_die("system'core'leave");
    hU8  = head_find_abs_or_die("system'core'U8");
    hU16 = head_find_abs_or_die("system'core'U16");
    hU32 = head_find_abs_or_die("system'core'U32");
    
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
  U32 ret=    meow_invoke(lay->data_base); //var is in data!
//printf("call_meow: %08X\n",ret);

}
void interpret_commit(){
    var->run_ptr = var->data_ptr;
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
    data_compile_token(head_find_abs_or_die("system'core'STR8"));
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
extern HINDEX H_SYSVAR;
//TODO refprim...

/* ==========================================================
  handle source attachment
*/
//defining
int interpret_copy_source(HINDEX h){
    src_ws(); //
    head_append_source(h,var->src_ptr,0); //append this first line
    //now read lines and append as source until a line containing only "end"
    while(1){
        char*psrc = src_line();         //a fresh line of source
        U32 len = strlen(psrc);      
//printf("interpret_copy_source [%.*s] %d\n",len,psrc,len);
        //terminate source with a line containing an |
        head_append_source(h,psrc,len);
        if( (0==strncmp("end",psrc,3)) && (src_is_ws(psrc[3])) ){
            var->src_ptr += len;
            break;
        }
    }
    head_commit(h);
//printf("interpret_def_source done...[%s]\n",head_get_source(h));

}
void def_error(char*src, char*errptr){
    //print the portion of the source that is good
    color(COLOR_RESET); color(FORE_GREEN);
    printf("%.*s",(errptr-src),src);
    color(FORE_RED);
    printf("%s",errptr);
    color(COLOR_RESET);    
}


extern char* src_errbuf;
int interpret_def_PROC(HINDEX h){
    src_set(head_get_source(h));
    U32 ret = interpret_compuntil("end",3);
    if(!ret) {
        table_wipe(var->run_table);

printf("interpret_def_type: ret [%d]\n",ret);
printf("-----------\n");
def_error(head_get_name(h),src_errbuf);//var->src_ptr);
printf("-----------\n");
        return 0;
    }
    return 1;
}
int interpret_def_U32(HINDEX h){
    data_compile_U32(dstack_pop());
    interpret_commit();
//printf("interpret_def_U32 [%s]\n",var->src_ptr);
    return 1;
}
/* ==========================================================
  Sysvar has no data allocated - it points to some other data...
*/
int interpret_def_SYSVAR(HINDEX h){
//printf("SYSVAR [%s]\n",var->src_ptr);
    //the pointer is on the datastack!
    TOKEN* ptr = (TOKEN*)dstack_pop();
    head_set_code(h,ptr);         
   
    return 1;
}

int interpret_def_type(HINDEX htype){
//printf("interpret_def_type: type [%.*s]\n",head_get_namelen(htype),head_get_name(htype));
    //TODO: check for duplication...of string and of datatptr...
    HINDEX h = head_new(var->data_ptr,  htype,var->wd);

//printf("interpret_def_type1 [%s]\n",var->src_ptr);
    interpret_copy_source(h); //in any event, append source and commit entry
//printf("interpret_def_type2 [%s]\n",var->src_ptr);
//printf("interpret_def: src [%s]\n",var->src_ptr);
//printf("lang_colon: type %.*s %d \n",head_get_namelen(htype),head_get_name(htype));
    //dispatch on type! Note: tcnt and tname are not valid, as source has changed
    int ret=0;
    if(htype == H_PROC) {
        ret = interpret_def_PROC(h);
    }
    else if(htype == H_U32)
        ret = interpret_def_U32(h);
    else if(htype == H_SYSVAR)
        ret = interpret_def_SYSVAR(h);
    else {
        //-------------------------------------------
        // unimplemented type
        U32 n; const char*p = head_name(h,&n);
        printf("def_type invalid type %.*s:\n",n,p);
    }
//U32 n; const char*p = head_name(h,&n);
//printf("interpret_def_type: done %.*s:\n",n,p);
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
        //COMPILING!
        //simple type dispatch.  Real language dispatches better..
        HINDEX type = head_get_type(h); //TYPE encountered? define!
        if(type==H_TYPE)
            return interpret_def_type(h);
        if(type==H_PROC)
            return data_compile_token(h);                  //compile a token...
        if(type==H_U32){
            return data_ref_style(h,"system'TYPE'U32'fetch");
        }
        if(type==H_SYSVAR){
            return data_ref_style(h,"system'TYPE'U32'fetch");
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
//            printf("compuntil...ERROR [[%s]]\n",var->src_ptr);
            return 0;
        }
         
    }
    return 1;
}

int command(char* ptr,U32 cnt);


int interpret_outer_p(char* ptr,U32 cnt){
//printf("OUTER [%.*s]\n",cnt,ptr);
    if(!command(ptr,cnt)) 
        if(!interpret_compone(ptr,cnt)) {        //otherwise, do the magic
            src_error("not found:");
            *var->src_ptr=0;                         //abandon line!
            return 0;
         }
//printf("OUTER done\n");
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
//table_dump( ((U32)(var->run_ptr)) >>4 << 2);
        call_meow(var->run_ptr);                    //run from run_ptr
//printf("interpreter-outer done\n");
//    }      
    //---------------------------------------
    // reset after run
    memset(var->run_ptr,0xFF,(var->data_ptr - var->run_ptr));
//printf("out: run_table is at %08p\n",var->run_table);
    var->data_ptr = var->run_ptr;               //and reset
    table_wipe(var->run_table);
    return 1;
   
}

