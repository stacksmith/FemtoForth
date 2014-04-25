#include "global.h"
#include "header.h"
#include "src.h"
#include "interpret.h"
#include "cmd.h"

extern sHeader*       HEAD;
extern char* src_ptr;           //from src.cpp
// interpret.c

#include "cmd.h"
extern HINDEX search_list[];

 sVar* var;
/*
typedef U32 (*funcx)(sVar* v,U32 parm);
U32 interpret_xxx(HINDEX h){
    funcx p = (funcx)var->table_base[HEAD[h].index];
printf("interpret_xxx: %d %s %p\n",h,&HEAD[h].name,p);
    interpret_ql((void*)p);
    U32 ret = p(var,'q');
printf("interpret_xxx: %08x %d\n",ret,ret);
 
    
}

void call_meow(){
  U32 ret=    meow_invoke(var);
printf("call_meow: %08X\n",ret);
    
}
*/
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
    var->sp_meow = p;
    // On another note, initialize the return hindex..
    hreturn= head_find_absolute("core';",6);
    hleave = head_find_absolute("core'leave",10);
    hU8  = head_find_absolute("core'U8",7);
    hU16 = head_find_absolute("core'U16",8);
    hU32 = head_find_absolute("core'U32",8);
}
/* ============================================================================
 * comp  Compile a token representing header index.
 * 
 * This is hard.  Compile a token.  Its address is transformed into table base
 * which is searched for the target. If target is found, a token is calculated
 * from that table index.  If not, we attempt to insert a new target in this 
 * range.
 */

int interpret_comp(HINDEX h){
//printf("interpret_comp: %d %s \n",h,&HEAD[h].name);
    //get table base for this location+1
    //+4 since index 0 is used for 'code'
    U8**tbase = (U8**)(((U32)(var->data_ptr+1) >>2) & 0xFFFFFFFC);
//printf("interpret_comp to: %08X, base %08X\n",var->data_ptr+1,tbase);
    //now, in the range of 1-255, try to find the entry represented by
    //HINDEX h...  
    U8* target = HEAD[h].pcode; //that's what HINDEX h targets...
    int tok;
    for(tok=1;tok<=255;tok++){ //for every possible token value
        if(target==tbase[tok]) { //does the table already have a reachable?
          *var->data_ptr++ = tok; //compile token
//printf("interpret_comp: found an entry, compiled token %02x at %08x\n",tok,var->data_ptr-1);
           return 1;
        } else {
            if(NULL==tbase[tok]) { //empty slot?
             extern HINDEX H_PROC;           //initilization code set this...
   tbase[tok] = target;    //create a slot entry
                *var->data_ptr++ = tok; //compile token
//printf("interpret_comp: created an entry, compiled token %02x at %08x\n",tok,var->data_ptr-1);
                return 1;
            }   
        }
    }
    // There was not a single empty slot in the reachable part of the table...
    printf("interpret_comp: ERROR - no empty space...\n");
    return 0;
    
}

void call_meow(U8* addr){
//printf("call_meow will run: %08X\n",addr);
    sRegsMM* pregs = (sRegsMM*)var->sp_meow;
   pregs->IP = (U32)addr;
  U32 ret=    meow_invoke(var);
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
        interpret_comp(hU8);
        data_compile_U8(val);
    } else if(!(val&0xFFFF0000)){
        interpret_comp(hU16);
        data_compile_U16(val);
    } else {
        interpret_comp(hU32);
        data_compile_U32(val);
    }
    
    return 1;
}
int interpret_literal_c(char* ptr,U32 cnt){
//printf("interpret_literal_c [%s] %x %d\n",ptr,ptr,cnt);
    if((cnt==3)&&('\''==*(ptr+2))){ //sanity check
        interpret_comp(hU8);
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
    if(x) return interpret_comp(x);                  //compile a token...
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
    //keep track of where we started...
    var->run_ptr = var->data_ptr;
    var->run_table = var->table_ptr;
    //try to run as command
    if(!command(ptr,cnt)) 
        if(!interpret_compone(ptr,cnt)) {        //otherwise, do the magic
            src_error("not found:");
            *src_ptr=0;                         //abandon line!
            return 0;
         }
    //execute
    if(var->run_ptr != var->data_ptr) {
        interpret_comp(hleave);                    //terminate with a return
cmd_ql(var->run_ptr);
        call_meow(var->run_ptr);                    //run from run_ptr
    }      
    // reset run space
    memset(var->run_ptr,0xFF,(var->data_ptr-var->run_ptr));
    var->data_ptr = var->run_ptr;               //and reset
    memset(var->run_table,0x00,sizeof(U8*) * (var->table_ptr - var->run_table));
    var->table_ptr = var->run_table;
    return 1;
   
}

