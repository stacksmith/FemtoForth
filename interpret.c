#include "global.h"
#include "header.h"
#include "interpret.h"

extern sHeader*       HEAD;
// interpret.c
char src_buf[256]="";
char* src_ptr = src_buf;
char* src_saveptr=0;

/* TODO: fix this, pls */
typedef struct sInterp {
  HINDEX list[16];
}sInterp;

sInterp icontext = {4,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};




void src_line(){
    gets(src_buf);
    src_ptr = src_buf;
}
void src_ws(){
  while(1){
    char c = *src_ptr++;
    switch(c){
      case ' ': break;
      case '\t': break;
      case 0x0D: break;
      case 0x0A: break;
      case 0: src_line(); break;
      default: src_ptr--; return;
    }
  }
}

U32 src_cnt(){
  U32 cnt=0;
  char*p = src_ptr;
  while(1){
    char c = *p++;
    switch(c){
      case ' ': return cnt;
      case '\t': return cnt;
      case 0x0D: return cnt;
      case 0x0A: return cnt;
      case 0: return cnt;
      default: 
        cnt++;
    }
  }
}

U32 src_one(){
  src_ws();
  return src_cnt();
}

interpret_ls(HINDEX dir){
printf("ls in %d\n",dir);
  HINDEX h = HEAD[dir].child;
  while(h){
    printf("%s\n",HEAD[h].name);
    h=HEAD[h].next;
  } 
}
//
// cd
//
int interpret_cd(){
  U32 cnt = src_one();
  char* ptr = src_ptr;
  src_ptr += cnt;
  HINDEX x=1; //root
  // handle cd '  as cd to root...
  if(  ((cnt==2)&&(*ptr=='.')&&(*(ptr+1)=='.') )){
    x = HEAD[icontext.list[0]].dad;
  } else  
  if( ! ((cnt==1)&&(*ptr=='\'')) )
     x = head_find(ptr,cnt,icontext.list);
  // handle cd .. as cd up
  if(x) {
    icontext.list[0]=x;
    return 1;
  } else{
    printf("ERROR: cd could not cd to [%s]\n",ptr);
    return 0;
  }
}
//
// q
//
U8* interpret_ql(U8*p){
  printf("%08X ",p);
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

int interpret_q(){
  U32 cnt = src_one();
  char* ptr = src_ptr;
  src_ptr += cnt;
  U8* address = (U8*)strtol(ptr,NULL,16);
  address = interpret_ql(address);
  address = interpret_ql(address);
  address = interpret_ql(address);
  address = interpret_ql(address);
  return 1;
}
extern sVar* var;
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
    hreturn= head_find_absolute(irpstr1,8);
    hleave = head_find_absolute(irpstr2,strlen(irpstr2));

}

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
    sRegsMM* pregs = var->sp_meow;
   pregs->r6 = addr;
  U32 ret=    meow_invoke(var);
printf("call_meow: %08X\n",ret);
    
}

extern HINDEX H_PROC;           //initilization code set this...
void interpret_colon(){
  U32 cnt = src_one();
  char* ptr = src_ptr;
  src_ptr += cnt;
//printf("interpret_colon: will create %s\n",ptr);
    //TODO: check for duplication...of string and of datatptr...
    head_new(ptr,cnt, var->data_ptr,  H_PROC,T_NA, icontext.list[0]);
  
}

int interpret_compone(char* ptr,U32 cnt){
//printf("interpret_compone[%s] %d\n",ptr,cnt);
    HINDEX x = head_find(ptr, cnt,icontext.list);
    if(!x) {
        printf("interpret_compone:not found %s %d\n",ptr,cnt);     
        return 0;
    }
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
int interpret_command(char* ptr,U32 cnt){
    switch(cnt){
        case 1:
            if(0==strncmp(ptr,":",1)) { interpret_colon(); return 1;}
            if(0==strncmp(ptr,"(",1)) { interpret_compuntil(")",1); return 1;}
            if(0==strncmp(ptr,"{",1)) { interpret_compuntil("}",1);
              var->run_ptr = var->data_ptr; return 1;}
            
        case 2:
            if(0==strncmp(ptr,"ls",2)) { interpret_ls(icontext.list[0]);return 1; }
            if(0==strncmp(ptr,"cd",2)) { interpret_cd(); return 1;  };
            if(0==strncmp(ptr,"_q",2)) { interpret_q(); return 1;}
        case 3:
            if(0==strncmp(ptr,"pwd",3)) { printf("%s\n",HEAD[icontext.list[0]].name); return 1;}
            if(0==strncmp(ptr,"run",3)) { call_meow(var->run_ptr); return 1;}
        case 4:
            if(0==strncmp(ptr,"exit",4)) {exit(0);}
    }
    return 0;
}


int interpret_one(){

    U32 cnt = src_one();
    char* ptr = src_ptr;
    src_ptr += cnt;

    //prepare to run code we are about to compile!
    var->run_ptr = var->data_ptr;

    //try to run as command
    if(! interpret_command(ptr,cnt)) 
        interpret_compone(ptr,cnt);             //otherwise, do the magic
    interpret_comp(hleave);                    //terminate with a return
    
interpret_ql(var->run_ptr);
    call_meow(var->run_ptr);                    //run from run_ptr
    
    var->data_ptr = var->run_ptr;               //and reset
    
    return 1;
   
}

