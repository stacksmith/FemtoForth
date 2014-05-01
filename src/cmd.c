#include "global.h"
#include "header.h"
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
  HINDEX h = head_get_child(dir);
  while(h){
    printf("\33[0;32m");
    int dir = (head_get_child(h) != 0);        //TODO:
    if(dir) printf("\33[1;32m");
    printf("%.*s ",head_get_namelen(h),head_get_name(h));
    if(dir) printf("\33[0;37m");
    //now the comment part
    printf("\t\33[0;33m %s\n",head_get_name(h)+head_get_namelen(h));
    h=head_get_next(h);
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
  HINDEX x=head_get_root(); //root
  // handle cd '  as cd to root...
  if(  ((cnt==2)&&(*ptr=='.')&&(*(ptr+1)=='.') )){
    x = head_get_dad(search_list[0]);
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
    printf("\33[1;32m     TOS      NOS      DSP      RSP     HEAD      CPL     BASE  ENTRIES\33[0;32m\n");
    U32* DSP = (U32*)(var->sp_meow->DSP);
    printf("%08X ",var->sp_meow->TOS);          //TOS
    printf("%08X ",*DSP);                       //NOS
    printf("%08X ",(U32)DSP);                   //DSP
    printf("%08X ",(U32)(var->sp_meow+1));      //RSP (not counting struct)
    printf("%08X ",(U32)(var->head_ptr));
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
 * 
 * Headers
 * -------
 * Internal header structural references are all indices, no fixup.
 * References into data section are actual pointers...
 * 
 * Table
 * -----
 * A list of pure pointers.  
 * -The low 8 pointers are filled out by loader...
 * -5 pointers follow, that require special care.
 * 
 * CONTEXT
 * ==========================================================================*/
int cmd_save(){
    FILE* f = fopen("femto_image.data","w");
    data_save(f);
    fclose(f);
    
    f = fopen("femto_image.head","w");
    head_save(f);
    fclose(f);
    
    return 1;
}
TOKEN* deco_one(TOKEN*p){
    PTOKEN* base = table_base(p);
    U8 token = *p;
    if(!token){
        printf("%p ;\n",p);
        return(p++);
    }
    
    
    PTOKEN target = base[*p];
    
    U32 off;
    HINDEX head = head_resolve(target,&off);
    printf("%p %.*s",p,head_get_namelen(head),head_get_name(head));
    p++;
    U32 type;
    switch(type=head_get_parm(head)){
        case T_NA:
            break;
        case T_U8: 
            printf(" %02X",*(U8*)p);
            p++;
            break;
        case T_U16: 
            printf(" %04X",*(U16*)p);
            p++;
            break;
        case T_U32:
            printf(" $%08X (%d)",*(U32*)p,*(U32*)p);
            p+=4;
            break;
        case T_OFF:
            printf(" %p (%d)",p + 1 + *(S8*)p,*(S8*)p);
            p++;
            break;
        default:
            printf("UNIMPLEMENTED type in decompiler %d\n",type);
            exit(0);
    }
    
    printf("\n");
    return p;
}
int cmd_deco(){
    TOKEN* ptr = (TOKEN*)(var->sp_meow->TOS);
    //validate token
    if ((ptr < var->data_base) || (ptr > var->data_top)){
        src_error("deco: illegal address\n");
        return 0;
    }
    int i;
    for(i=0;i<15;i++){
        ptr = deco_one(ptr);
    }
}

int cmd_load(){
    return src_file("test.ff");
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
            if(0==strncmp(ptr,"ok",2)) { return 1;  };
            break;
        case 3:
            if(0==strncmp(ptr,"pwd",3)) { printf("%s\n",head_get_name(search_list[0])); return 1;}
            if(0==strncmp(ptr,"run",3)) { call_meow(var->run_ptr); return 1;}
            if(0==strncmp(ptr,"sys",3)) { return cmd_sys();}
            break;
        case 4:
            if(0==strncmp(ptr,"exit",4)) {exit(0);}
            if(0==strncmp(ptr,"save",4)) {return cmd_save();}
            if(0==strncmp(ptr,"deco",4)) {return cmd_deco();}
            if(0==strncmp(ptr,"load",4)) {return cmd_load();}
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
    search_list[0] = head_find_abs_or_die("test");    //wd
    search_list[1] = head_find_abs_or_die("core");    //
    search_list[2] = head_find_abs_or_die("io");    //
}
