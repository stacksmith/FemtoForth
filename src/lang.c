#include <stdio.h>
#include <string.h>

#include "global.h"
#include "header.h"
#include "src.h"
extern char* src_ptr;           //from src.cpp
#include "interpret.h"
extern sVar* var;
#include "cmd.h"
extern HINDEX search_list[];
#include "data.h"


//TODO: check error conditions, return...
extern HINDEX H_PROC;           //initilization code set this...

//HINDEX H_OPDIR;          
HINDEX H_0BRANCH;
HINDEX H_BRANCH;
void lang_init(){
    H_BRANCH = head_find_abs_or_die("core'branch");
    H_0BRANCH = head_find_abs_or_die("core'0branch");
}

int lang_colon(){
    //the entire line is name/comment...
//  U32 cnt = src_one();
//  char* ptr = src_ptr;
//  src_ptr += cnt;
//printf("interpret_colon: will create %s\n",ptr);
    //TODO: check for duplication...of string and of datatptr...
    src_ws(); //TODO: check for immediate return case...
    U32 cnt = strlen(src_ptr);
    head_new(src_ptr,cnt, var->data_ptr,  H_PROC,T_NA, search_list[0]);
    src_ptr +=cnt;
    return 1;
}
int verify_ptr(U8* ptr){
    if ((ptr >= var->data_base) && (ptr < var->data_top))     return 1;
    if ((ptr >= var->table_base) && (ptr < var->table_top))   return 1;
    if ((ptr >= var->dsp_base) && (ptr < var->dsp_top))       return 1;
    if ((ptr >= var->rsp_base) && (ptr < var->rsp_top))     return 1;
    if ((ptr >= var->head_base) && (ptr < var->head_top))     return 1;
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

  U32 cnt = src_one();
  char* ptr = src_ptr;
  src_ptr += cnt;
  U8* address = (U8*)strtol(ptr,NULL,16);

//    U8* address = (U8*)(var->sp_meow->TOS);
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
   HINDEX hpush = head_find_abs_or_die("core'push");
   data_compile_token(hpush);
   dstack_push((U32)var->data_ptr);       //save loop target on stack
   interpret_one();                     //compile expression
   HINDEX htimes = head_find_abs_or_die("core'times");
   data_compile_token(htimes);
   data_compile_off_S8(dstack_pop());
   

}
int lang_t(){
    PTOKEN* p = (PTOKEN*) dstack_pop();
    table_dump(p);
}

int lang_ref(char* ptr,U32 cnt){
    //find the identifier following the &
    HINDEX htarget = head_find(ptr,cnt,search_list);
    if(!htarget) return 0; //TODO: error
    HINDEX href = head_find_absolute("core'REF",8);
    if(!href) return 0;
    data_compile_token(href);
    data_compile_token(htarget);
    return 1;
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
int lang_if_paren(){
    int ret = interpret_compuntil(")",1);       //compile conditional
    
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
int lang_p(char* ptr,U32 cnt){
// printf("lang_p: [%.*s] %d\n", cnt,ptr,cnt);
   
    
    // If a word starts with a &, get the address of it...
    // and compile it 
    if((cnt>1)&&('&'==*ptr)){
        return lang_ref(ptr+1,cnt-1);
    }
    switch(cnt){
        case 1:
//            if(0==strncmp(ptr,"+",1)) { return lang_operator(ptr,cnt); }
//            if(0==strncmp(ptr,"&",1)) { return lang_ref); }
            if(0==strncmp(ptr,";",1)) { return lang_return(); }
            if(0==strncmp(ptr,":",1)) { return lang_colon(); }
            if(0==strncmp(ptr,"q",1)) { return lang_q(); }
            if(0==strncmp(ptr,"t",1)) { return lang_t(); }
            if(0==strncmp(ptr,"(",1)) { return interpret_compuntil(")",1);}
          
            break;
        case 2:
             if(0==strncmp(ptr,"if",2)) { return lang_if(); }
             if(0==strncmp(ptr,"//",2)) { src_skip_line(); return 1;}
             
           
            break;
        case 3:
            if(0==strncmp(ptr,"if(",3)) { return lang_if_paren(); }
            break;
        case 4:
             if(0==strncmp(ptr,"else",4)) { return lang_else(); }
            break;
        case 5:
            if(0==strncmp(ptr,"times",5)) { return lang_times(); }
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
    if('('==ptr[cnt-1]){
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
