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


//TODO: check error conditions, return...
extern HINDEX H_PROC;           //initilization code set this...
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
   HINDEX hpush = head_find_absolute("core'push",9);
   data_compile_token(hpush);
   dstack_push((U32)var->data_ptr);       //save loop target on stack
   interpret_one();                     //compile expression
   HINDEX htimes = head_find_absolute("core'times",10);
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
int lang(char* ptr,U32 cnt){
//printf("lang [%s] %x %d\n",ptr,ptr,cnt);
    // If a word starts with a &, get the address of it...
    // and compile it 
    if((cnt>1)&&('&'==*ptr)){
        return lang_ref(ptr+1,cnt-1);
    }
    switch(cnt){
        case 1:
            if(0==strncmp(ptr,";",1)) { return lang_return(); }
            if(0==strncmp(ptr,":",1)) { return lang_colon(); }
            if(0==strncmp(ptr,"q",1)) { return lang_q(); }
            if(0==strncmp(ptr,"t",1)) { return lang_t(); }
            if(0==strncmp(ptr,"(",1)) { return interpret_compuntil(")",1);}
          
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            break;
        case 5:
            if(0==strncmp(ptr,"times",5)) { return lang_times(); }
            break;
    }
    return 0;
}
