#include "global.h"
#include "header.h"
extern sHeader*       HEAD;
#include "src.h"
extern char* src_ptr;           //from src.cpp
// interpret.c

#include "cmd.h"
sInterp icontext = {4,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};




extern sVar* var;



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

extern HINDEX H_PROC;           //initilization code set this...
void interpret_colon(){
  U32 cnt = src_one();
  char* ptr = src_ptr;
  src_ptr += cnt;
//printf("interpret_colon: will create %s\n",ptr);
    //TODO: check for duplication...of string and of datatptr...
    head_new(ptr,cnt, var->data_ptr,  H_PROC,T_NA, icontext.list[0]);
  
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

