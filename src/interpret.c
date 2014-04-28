#include <string.h>
#include "global.h"
#include "header.h"
#include "src.h"
#include "interpret.h"
#include "cmd.h"
#include "table.h"
#include "lang.h"

extern char* src_ptr;           //from src.cpp
// interpret.c

#include "cmd.h"
extern HINDEX search_list[];

 sVar* var;

/* ==========================================================
 * Initialize the register contexts...
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
  U32 ret=    meow_invoke(var);
printf("call_meow: %08X\n",ret);

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
int interpret_literal_c(char* ptr,U32 cnt){
//printf("interpret_literal_c [%s] %x %d\n",ptr,ptr,cnt);
    if((cnt==3)&&('\''==*(ptr+2))){ //sanity check
        data_compile_token(hU8);
        data_compile_U8(*(ptr+1));
        return 1;
    }
    return 0;
}
int interpret_literal(char* ptr,U32 cnt){
//printf("interpret_literal [%s] %x %d\n",ptr,ptr,cnt);
    U32 radix = 10;
    char first = *ptr;
    switch(first){
        case '$': return interpret_literal_num(ptr+1,cnt-1,16);
        case '\'': return interpret_literal_c(ptr,cnt);
//        case '"': return interpret_literal_str(ptr,cnt);
        default:  return interpret_literal_num(ptr,cnt,10);
    }
    return 0;
}
extern int lang(char*ptr,U32 cnt);
/*
 * compile a single unit - lang, native or literal...
 */
int interpret_compone(char* ptr,U32 cnt){
//printf("interpret_compone[%s] %d\n",ptr,cnt);
    //check for 'x' literals, they will break head_find!
    if(('\''==*ptr)&&('\'')==*ptr+2) return  interpret_literal_c(ptr,cnt);
    if(lang(ptr,cnt)) return 1;

    HINDEX x = head_find(ptr, cnt,search_list);
    if(x) return data_compile_token(x);                  //compile a token...
    return interpret_literal(ptr,cnt);        //finally try literal
}

int interpret_one(){
    //next
    U32 cnt = src_one();
    char* ptr = src_ptr;
    src_ptr += cnt;
    return interpret_compone(ptr,cnt);
}


int interpret_compuntil(char* delim, U32 delimcnt){
    while(1){
        U32 cnt = src_one();
        char* ptr = src_ptr;
        src_ptr += cnt;
        if((delimcnt==cnt)&&(0==strncmp(delim,ptr,cnt)))
            return 1;
        if(!interpret_compone(ptr,cnt)) {
            printf("ERROR [[%s]]\n",src_ptr);
            return 0;
        }
         
    }
    return 1;
}

int command(char* ptr,U32 cnt);



int interpret_outer(){

    U32 cnt = src_one();
    char* ptr = src_ptr;
    src_ptr += cnt;
    //---------------------------------------
    //Prepare to clean up our tracks...
    var->run_ptr = var->data_ptr;
    var->run_table = table_end(var->data_ptr);
//printf("in: run_table is at %08p\n",var->run_table);
    //try to run as command
    if(!command(ptr,cnt)) 
        if(!interpret_compone(ptr,cnt)) {        //otherwise, do the magic
            src_error("not found:");
            *src_ptr=0;                         //abandon line!
            return 0;
         }
    //execute
//    if(var->run_ptr != var->data_ptr) {
//        data_compile_U8(0);
        data_compile_token(hleave);                    //terminate with a return
//printf("--%p\n",var->data_ptr);
lang_ql(var->run_ptr);//
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

