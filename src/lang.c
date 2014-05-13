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
#include <stdio.h>
#include <string.h>

#include "global.h"
#include "header.h"
#include "src.h"

#include "interpret.h"
extern sVar* var;
extern sMemLayout* lay;
#include "cmd.h"
extern HINDEX search_list[];
#include "data.h"
#include "table.h"

//TODO: check error conditions, return...
extern HINDEX H_PROC;           //initilization code set this...

//HINDEX H_OPDIR;          
HINDEX H_0BRANCH;
HINDEX H_BRANCH;
void lang_init(){
    H_BRANCH = head_find_abs_or_die("system'core'branch");
    H_0BRANCH = head_find_abs_or_die("system'core'0branch");
}

int verify_ptr(U8* ptr){
    if ((ptr >= lay->data_base) && (ptr < lay->data_top))     return 1;
    if ((ptr >= lay->table_base) && (ptr < lay->table_top))   return 1;
    if ((ptr >= lay->dsp_base) && (ptr < lay->dsp_top))       return 1;
    if ((ptr >= lay->rsp_base) && (ptr < lay->rsp_top))     return 1;
    if ((ptr >= lay->head_base) && (ptr < lay->head_top))     return 1;
    return 0;
}
/*=============================================================================
 * q
 * 
 * dump pointer on datastack...
 * ==========================================================================*/
//
// q
//
U8* lang_ql(U8*p){
  printf("%p ",p);
  int i;
  for(i=0;i<16;i++){
    printf("%02X ",p[i]);
  }
  for(i=0;i<16;i++){
    if(isprint(p[i]))
      printf("%c",p[i]);
    else
      printf("%s","·");
  }
  printf("\n");
  return p+16;
}
int lang_q(){
  printf("\33[0;32m");

/*  U32 cnt = src_one();
  char* ptr = var->src_ptr;
  var->src_ptr += cnt;
  U8* address = (U8*)strtol(ptr,NULL,16);
*/
    U8* address = (U8*)(var->sp_meow->TOS);
    if(!verify_ptr(address+64)){
        src_error("q: illegal address\n");
        return 0;
    }
// printf("SP_MEOW ADDRESS %p\n",address);
  address = lang_ql(address);
  address = lang_ql(address);
  address = lang_ql(address);
  address = lang_ql(address);
  var->sp_meow->TOS = (U32)address;
  printf("\33[0;37m");
  return 1;
}


void dstack_push(U32 val){
    *(--var->sp_meow->DSP)=var->sp_meow->TOS;
    var->sp_meow->TOS = val;
}
U32 dstack_pop(){
    U32 ret = var->sp_meow->TOS;
    var->sp_meow->TOS = *(var->sp_meow->DSP++);
    return ret;
}

void dstack_write(U32 val){
    var->sp_meow->TOS=val;
}

U32 dstack_read(){
    return (var->sp_meow->TOS);
}

/*=============================================================================
 * times
 * 
 * n times ( )
 * 
 * push n onto return stack
 * compile next expression
 * compile loop, decrementing RSP count and looping expression.
 * pop count off the return stack.
 * ==========================================================================*/
int lang_times(){
    HINDEX hpush = head_find_abs_or_die("system'core'push");
    data_compile_token(hpush);
    dstack_push((U32)var->data_ptr);       //save loop target on stack
    if(interpret_one()) {                     //compile expression
        HINDEX htimes = head_find_abs_or_die("system'core'times");
        data_compile_token(htimes);
        data_compile_off_S8(dstack_pop());
        return 1;
    }
    return 0;
   

}
int lang_t(){
    PTOKEN* p = (PTOKEN*) dstack_pop();
    dstack_push( table_dump(p) );
}


int lang_return(void){
    data_compile_U8(0);
    return 1;
}
void lang_fixup(){
    U8* pfix = (U8*)dstack_pop();       //recover fixup address
    U32 offset = var->data_ptr - pfix-1;
//printf("lang_fixup: offset is %d\n",offset);
    *pfix = offset;
}

int lang_if(){
    data_compile_token(H_0BRANCH);      //compile branch instruction
    dstack_push((U32)data_compile_U8(0));    //keep fixup on stack
    if(!interpret_compuntil("thanx",5)) {       //compile until 'then'
printf("lang_if: err \n");
        dstack_pop();  //on error, get rid of pfixup
        return 0;
    }
    lang_fixup();
    return 1;
}

int lang_else(){
    data_compile_token(H_BRANCH);      //compile branch instruction
    U8* newfixup = data_compile_U8(0);  //else's fixup address
    //fixup if's jump to here...
    lang_fixup();
    //leave our fixup for if
    dstack_push((U32)newfixup);
    
}
int lang_begin(){
    dstack_push((U32)var->data_ptr);    //just save the loop start location
    if(!interpret_compuntil("repeat",6)) {       //compile until 'again'
printf("lang_begin: err \n");
        dstack_pop();  //on error, get rid of pfixup
        return 0;
    }
    data_compile_token(H_BRANCH);
    TOKEN* target = (TOKEN*)dstack_pop();       //recover loop start
    U32 offset = target - var->data_ptr - 1;
    data_compile_U8(offset);
    return 1;
    
}

/*int lang_operator(char* ptr,U32 cnt){
    HINDEX hop = head_locate(H_OPDIR,ptr,cnt);
    if(!hop){
        src_error("operator not found");
        return 0;
    }
    // Compile RHS
    interpret_one();
    // now compile operator
    data_compile_token(hop);
    return 1;
}
*/
int lang_oper(char* opname,U32 oplen){
//printf("lang_oper\n");
    U32 cnt; char* ptr = src_word(&cnt);        //next
    HINDEX h = head_find(ptr,cnt,search_list);  //find the subject of operation
    if(!h) return 0;
    HINDEX type = head_get_type(h);
/*    HINDEX prim = head_locate(type,"prim",4);
    if(!prim) {
        printf("lang_oper can't find TYPE'%.*s'prim directory\n",
               head_get_namelen(type),head_get_name(type));
        return 0;
    }
*/    HINDEX operation = head_locate(type,opname,oplen);
    if(!operation){
        printf("lang_oper can't find TYPE'%.*s'%.*s directory\n",
               head_get_namelen(type),head_get_name(type),oplen,opname);
        return 0;
    }
    //compile a reference-style sequence
    return data_ref_style_p(h,operation);
}
/*=============================================================================
* ref   - compile a reference to an address, using the table (via a token)
* 
* - code: <ref> <tok>   at runtime leave address (onding to tok) on dstack
*/

int lang_ref(){
    U32 cnt; char* ptr = src_word(&cnt);        //next
    HINDEX h = head_find(ptr,cnt,search_list);  //find the subject of operation
    //now compile as a tabled variable, to allow for relocation   
    return data_ref_style(h,"system'core'REF");
}
int lang_head(){
    U32 cnt; char* ptr = src_word(&cnt);        //next
    HINDEX h = head_find(ptr,cnt,search_list);  //find the subject of operation
    //compile a reference to the actual header (not its data...)
    data_compile_token(head_find_abs_or_die("system'core'REF")); //
    data_compile_ref((U8*)h);   
    return 1;
}
int lang_p(char* ptr,U32 cnt){
// printf("lang_p: [%.*s] %d\n", cnt,ptr,cnt);
   
    //--------------------------------------------------
    // address of operator
//    if((cnt>1)&&('&'==*ptr)){
//        return lang_ref(ptr+1,cnt-1);
//    }
    switch(cnt){
        case 1:
//            if(0==strncmp(ptr,"+",1)) { return lang_operator(ptr,cnt); }
//            if(0==strncmp(ptr,"&",1)) { return lang_ref); }
            if(0==strncmp(ptr,";",1)) { return lang_return(); }
            if(0==strncmp(ptr,"q",1)) { return lang_q(); }
            if(0==strncmp(ptr,"t",1)) { return lang_t(); }
            if(0==strncmp(ptr,"(",1)) { return interpret_compuntil(")",1);}
            if(0==strncmp(ptr,"{",1)) {
                int ret = interpret_compuntil("}",1);
//printf("{...} %d\n",ret);
                if(ret){
                    //don't let interpreter erase us!
                    var->run_ptr = var->data_ptr;
//printf("update: run_table is at %08p\n",var->run_table);
                    var->run_table = table_end(var->data_ptr);
                    return 1;
                } else {
                    return 0;
                }
            }
         
            break;
        case 2:
             if(0==strncmp(ptr,"if",2)) { return lang_if(); }
             if(0==strncmp(ptr,"//",2)) { src_skip_line(); return 1;}
             
           
            break;
        case 3:
             if(0==strncmp(ptr,"ref",3)) { return lang_ref(); }
            break;
        case 4:
             if(0==strncmp(ptr,"else",4)) { return lang_else(); }
             if(0==strncmp(ptr,"into",4)) { return lang_oper("into",4); }
             if(0==strncmp(ptr,"head",4)) { return lang_head(); }
            break;
        case 5:
            if(0==strncmp(ptr,"times",5)) { return lang_times(); }
            if(0==strncmp(ptr,"begin",5)) { return lang_begin(); }
            break;
    }
    return 0;
}

int lang(char* ptr,U32 cnt){
   // operatoring: if any word ends with a (, compile rhs expression
    // until the ')', then run the word without the (...  This essentially
    // turns any word into a parse-ahead-style operator.  For instance,
    // a +( b -( somefunction )
    // see( ' myword )
    //
    
    HINDEX operator;
    if( (cnt>1) && ('('==ptr[cnt-1])){
        // Before parsing ahead, preserve the operator
        char buf[256];
        strncpy(buf,ptr,cnt-1);
        buf[cnt-1]=0;
        // Now, compile rhs expression
        int ret = interpret_compuntil(")",1);       //compile conditional
        if(!ret) return 0;
        //finally invoke the operator
        ret= interpret_compone(buf,cnt-1); //may be a lang or a forth word
        return ret;
    }
    return lang_p(ptr,cnt);
}
