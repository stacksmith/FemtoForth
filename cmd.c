#include "global.h"
#include "header.h"
extern sHeader*       HEAD;
#include "src.h"
extern char* src_ptr;           //from src.cpp
// interpret.c

#include "cmd.h"
HINDEX search_list[] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};


/******************************************************************/

extern sVar* var;



cmd_ls(HINDEX dir){
  printf("\33[0;32m");
//printf("ls in %d\n",dir);
  HINDEX h = HEAD[dir].child;
  while(h){
    int dir = (HEAD[h].child != 0);        //TODO:
    if(dir) printf("\33[1;32m");
    printf("%s ",HEAD[h].name);
    if(dir) printf("\33[0;37m");
    h=HEAD[h].next;
  } 
  printf("\33[0;37m");
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
    x = HEAD[search_list[0]].dad;
  } else  
  if( ! ((cnt==1)&&(*ptr=='\'')) )
     x = head_find(ptr,cnt,search_list);
  // handle cd .. as cd up
  if(x) {
    search_list[0]=x;
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
/*=============================================================================
 * sys
 * 
 * display pointers and data
 * ==========================================================================*/
int cmd_sys(){
    printf("\33[1;32m     TOS      NOS      DSP      RSP\33[0;32m\n");
    U32* DSP = (U32*)(var->sp_meow->DSP);
    printf("%08X ",var->sp_meow->TOS);   //TOS
    printf("%08X ",*DSP);               //NOS
    printf("%08X ",(U32)DSP);                //DSP
    printf("%08X ",(U32)(var->sp_meow+1));       //RSP
    
    printf("\33[0;37m\n");
}


//TODO: check error conditions, return...
int command(char* ptr,U32 cnt){
    switch(cnt){
        case 1:
             if(0==strncmp(ptr,"(",1)) { return interpret_compuntil(")",1);}
            if(0==strncmp(ptr,"{",1)) {
                int ret = interpret_compuntil("}",1);
                if(ret){
                    var->run_ptr = var->data_ptr;
                    return 1;
                } else
                    return 0;
            }
            
        case 2:
            if(0==strncmp(ptr,"ls",2)) { cmd_ls(search_list[0]);return 1; }
            if(0==strncmp(ptr,"cd",2)) { interpret_cd(); return 1;  };
        case 3:
            if(0==strncmp(ptr,"pwd",3)) { printf("%s\n",HEAD[search_list[0]].name); return 1;}
            if(0==strncmp(ptr,"run",3)) { call_meow(var->run_ptr); return 1;}
            if(0==strncmp(ptr,"sys",3)) { return cmd_sys();}
        case 4:
            if(0==strncmp(ptr,"exit",4)) {exit(0);}
    }
    return 0;
}

/* ============================================================================
*  initialize
* 
*  Call this after the dictionary has been loaded
*/
void cmd_init(){
    search_list[0] = head_find_absolute("test",4);    //wd
    search_list[1] = head_find_absolute("core",4);    //
    search_list[2] = head_find_absolute("io",2);    //
}
