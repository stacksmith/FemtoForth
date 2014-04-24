#include "global.h"
#include "header.h"
#include "src.h"
#include "interpret.h"

extern sHeader*       HEAD;
extern char* src_ptr;           //from src.cpp
// interpret.c

#include "cmd.h"
extern sInterp icontext;

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
    sRegsMM* p = (((sRegsMM*)var->sp_meow)-1 );       
//printf("interpret_init: sp_meow at %08x \n",p);
    p->r0  = 0x9ABC;
    p->r6  = 0;             //IP will be set for the call
    p->r7  = (U32)var->dsp_top;  //DSP
    p->r9  = 0;             //exception register
    p->r11 = (U32)var;      //table
    p->lr  = (U32)&inner_interpreter; //defined in bindings
    var->sp_meow = (U8*)p;
    // On another note, initialize the return hindex..
    hreturn= head_find_absolute("system';",8);
    hleave = head_find_absolute("system'leave",12);
    hU8  = head_find_absolute("lit'U8",6);
    hU16 = head_find_absolute("lit'U16",7);
    hU32 = head_find_absolute("lit'U32",7);
}
/* ============================================================================
 * comp  Compile a token representing header index.
 * 
 * This is hard.  Compile a token.  Its address is transformed into table base
 * which is searched for the target. If target is found, a token is calculated
 * from that table index.  If not, we attempt to insert a new target in this 
 * range.
 */

void interpret_comp(HINDEX h){
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
           return;
        } else {
            if(NULL==tbase[tok]) { //empty slot?
             extern HINDEX H_PROC;           //initilization code set this...
   tbase[tok] = target;    //create a slot entry
                *var->data_ptr++ = tok; //compile token
//printf("interpret_comp: created an entry, compiled token %02x at %08x\n",tok,var->data_ptr-1);
                return;
            }   
        }
    }
    // There was not a single empty slot in the reachable part of the table...
    printf("interpret_comp: ERROR - no empty space...\n");
    
}

void call_meow(U8* addr){
//printf("call_meow will run: %08X\n",addr);
    sRegsMM* pregs = (sRegsMM*)var->sp_meow;
   pregs->r6 = (U32)addr;
  U32 ret=    meow_invoke(var);
printf("call_meow: %08X\n",ret);
    
}


int interpret_compone(char* ptr,U32 cnt){
//printf("interpret_compone[%s] %d\n",ptr,cnt);
    HINDEX x = head_find(ptr, cnt,icontext.list);
    if(!x) return 0;
    interpret_comp(x);                  //compile a token...
    return 1;
}
int interpret_compuntil(char* delim, U32 delimcnt){
    while(1){
        U32 cnt = src_one();
        char* ptr = src_ptr;
        src_ptr += cnt;
        if((delimcnt==cnt)&&(0==strncmp(delim,ptr,cnt)))
            return 1;
        interpret_compone(ptr,cnt);
    }
    return 1;
}
int interpret_literal(char* ptr,U32 cnt){
    char* endptr;
    U32 radix = 10;
    if('$'==*ptr){
        ptr++;cnt--;
        radix=16;
        
    }
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

int interpret_command(char* ptr,U32 cnt);
int interpret_one(){

    U32 cnt = src_one();
    char* ptr = src_ptr;
    src_ptr += cnt;

    //prepare to run code we are about to compile!
    var->run_ptr = var->data_ptr;

    //try to run as command
    if(!interpret_command(ptr,cnt)) 
        if(!interpret_compone(ptr,cnt))        //otherwise, do the magic
            if(!interpret_literal(ptr,cnt)) {        //finally try literal
                src_error("not found:");
                return 0;
            }
    interpret_comp(hleave);                    //terminate with a return
    
interpret_ql(var->run_ptr);
    call_meow(var->run_ptr);                    //run from run_ptr
    
    var->data_ptr = var->run_ptr;               //and reset
    
    return 1;
   
}

