#include "global.h"
#include "header.h"
extern sHeader*       HEAD;
#include "src.h"
extern char* src_ptr;           //from src.cpp
// interpret.c

#include "cmd.h"
HINDEX search_list[] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};

#include "table.h"

/******************************************************************/

extern sVar* var;



cmd_ls(HINDEX dir){
//printf("ls in %d\n",dir);
  HINDEX h = HEAD[dir].child;
  while(h){
    printf("\33[0;32m");
    int dir = (HEAD[h].child != 0);        //TODO:
    if(dir) printf("\33[1;32m");
    printf("%.*s ",HEAD[h].namelen,HEAD[h].pname);
    if(dir) printf("\33[0;37m");
    //now the comment part
    printf("\t\33[0;33m %s\n",HEAD[h].pname+HEAD[h].namelen);
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
/*=============================================================================
 * sys
 * 
 * display pointers and data
 * ==========================================================================*/
int cmd_sys(){
    printf("\33[1;32m     TOS      NOS      DSP      RSP      CPL     BASE  ENTRIES\33[0;32m\n");
    U32* DSP = (U32*)(var->sp_meow->DSP);
    printf("%08X ",var->sp_meow->TOS);          //TOS
    printf("%08X ",*DSP);                       //NOS
    printf("%08X ",(U32)DSP);                   //DSP
    printf("%08X ",(U32)(var->sp_meow+1));      //RSP (not counting struct)
    printf("%08X ",(U32)(var->data_ptr));       //compiling here
    printf("%08X ",(U32)(table_base(var->data_ptr)));  //table base
    printf("%8d ",(table_end(var->data_ptr) - (1+table_base(var->data_ptr))));  //table base
    
    printf("\33[0;37m\n");

//printf("%8d %8d\n",(table_end(var->data_ptr),(1+table_base(var->data_ptr))));  //table base
    
}
/*=============================================================================
 * save
 * 
 * save the current system.
 * 
 * Data Section
 * ------------
 * The contents are bytecode or pure data.  There are no pointers to be fixed.
 * Exception: the crappy pointers at the bottom!...
 * ==========================================================================*/
int cmd_save(){
}

//TODO: check error conditions, return...
int command(char* ptr,U32 cnt){
    switch(cnt){
        case 1:
             if(0==strncmp(ptr,"{",1)) {
                int ret = interpret_compuntil("}",1);
                if(ret){
                    //don't let interpreter erase us!
                    var->run_ptr = var->data_ptr;
//printf("update: run_table is at %08p\n",var->run_table);
                    var->run_table = table_end(var->data_ptr);
                    return 1;
                } else
                    return 0;
            }
            break;
           
        case 2:
            if(0==strncmp(ptr,"ls",2)) { cmd_ls(search_list[0]);return 1; }
            if(0==strncmp(ptr,"cd",2)) { interpret_cd(); return 1;  };
            break;
        case 3:
            if(0==strncmp(ptr,"pwd",3)) { printf("%s\n",HEAD[search_list[0]].pname); return 1;}
            if(0==strncmp(ptr,"run",3)) { call_meow(var->run_ptr); return 1;}
            if(0==strncmp(ptr,"sys",3)) { return cmd_sys();}
            break;
        case 4:
            if(0==strncmp(ptr,"exit",4)) {exit(0);}
            if(0==strncmp(ptr,"save",4)) {return cmd_save();}
            break;
            
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
