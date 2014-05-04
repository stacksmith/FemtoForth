/******************************************************************************
Copyright 2014 Victor Yurkovsky

This file is part of the FemtoForth project.

FPGAsm is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

FemtoForth is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with FemtoForth. If not, see <http://www.gnu.org/licenses/>.

*****************************************************************************/
#include "global.h"
#include "header.h"
#include "src.h"
extern sVar* var ;

#include "cmd.h"
HINDEX search_list[] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};

#include "table.h"
extern HINDEX H_U32;
extern HINDEX H_DIR;
extern HINDEX H_ROOT;
extern HINDEX H_TYPE;
#include "color.h"
/* ============================================================================
*  initialize
* 
*  Call this after the dictionary has been loaded
*/
void cmd_init(){
    search_list[0] = head_find_abs_or_die("test");    //wd
    search_list[1] = H_TYPE;    //
    search_list[2] = head_find_abs_or_die("core");    //
    search_list[3] = head_find_abs_or_die("io");    //
    search_list[4] = H_ROOT;    //
    
}

/******************************************************************/

extern sVar* var;


/*=============================================================================
  ls   list entries in wd

=============================================================================*/
cmd_ls(HINDEX dir){
//printf("ls in %d\n",dir);
  HINDEX h = head_get_child(dir);
  while(h){
    color(COLOR_RESET); color(FORE_GREEN);
//    printf("\33[0;32m"); //green
    int dir = (head_get_child(h) != 0);        //TODO:
    if(dir) {
        color(COLOR_BRIGHT); color(FORE_GREEN);
    }
    printf("%-10.*s ",head_get_namelen(h),head_get_name(h));
    // type
    HINDEX type = head_get_type(h);
    color(COLOR_NORMAL); color(FORE_WHITE);
    printf("%-6.*s",head_get_namelen(type),head_get_name(type));
    //now the comment part
    U32 comlen; char* com = head_get_comments(h,&comlen);
    color(COLOR_RESET); color(FORE_YELLOW);
    printf("%-64.*s\n",comlen,com);
    h=head_get_next(h);
  } 
  color(COLOR_RESET); color(FORE_WHITE);
}
/*=============================================================================
  cd  change working directory, parse ahead

=============================================================================*/
int cmd_cd(){
  U32 cnt = src_one();
  char* ptr = var->src_ptr;
  var->src_ptr += cnt;
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
  list  list source of an entry

=============================================================================*/
int cmd_list(){
  U32 cnt; char* ptr = src_word(&cnt);          //next word
  HINDEX h = head_find(ptr,cnt,search_list);
  if(!h) return 0;
  //output type
  HINDEX htype = head_get_type(h);
  color(COLOR_RESET);
  color(FORE_GREEN);
  printf("%.*s ",head_get_namelen(htype),head_get_name(htype));
  //now print the source
  color(FORE_WHITE);
  printf("%s\n",head_get_name(h));
  
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
TOKEN* see_one(TOKEN*p){
    PTOKEN* base = table_base(p);
    U8 token = *p;
    if(!token){
        printf("\33[2m<\33[0;32m;\33[0;2m>\33[0m");
        return(p++);
    }
    
    
    //print token
    PTOKEN target = base[*p];
    U32 off;
    HINDEX head = head_resolve(target,&off);
    //dim the brackets; token is green
    printf("\33[2m<\33[0;32m%.*s\33[0;2m>\33[0m",head_get_namelen(head),head_get_name(head));
    p++;
    U32 type=head_get_parm(head);
    if(T_NA != type){
        printf("\33[2m<\33[0;33m");      //dim brace, reddish consts
        switch(type){
            case T_NA:

                break;
            case T_U8: 
                printf("$%02X (%d)",*(U8*)p,*(U8*)p);
                if(isprint(*(char*)p))
                    printf(" '%c'",*(char*)p);
                p++;
                break;
            case T_U16: 
                printf("$%04X (%d)",*(U16*)p,*(U16*)p);
                p++;
                break;
            case T_U32:
                printf("$%08X (%d)",*(U32*)p,*(U32*)p);
                p+=4;
                break;
            case T_OFF:
                printf("(%d)",*(S8*)p); //p + 1 + *(S8*)p
                p++;
                break;
            case T_STR8:
                printf("\"%.*s\"",*(U8*)p,(char*)(p+1) );
                p=p+(*(U8*)p)+1;
                break;
            default:
                printf("UNIMPLEMENTED type in decompiler %d\n",type);
                printf("\33[0m");
                exit(0);
        }
        printf("\33[0;2m>\33[0m"); //crap background
    }
 
    
//    printf("\n");
    return p;
}
int cmd_see(){
//    U32 cnt = src_one();
//    char* p = var->src_ptr;
//    var->src_ptr += cnt;
    
//    HINDEX h = head_find(p,cnt,search_list);
//    if(!h) return 0;
//   TOKEN* ptr = h-
    
    
    
    TOKEN* ptr = (TOKEN*)dstack_pop();
    //validate token
    if ((ptr < var->data_base) || (ptr > var->data_top)){
        src_error("see: illegal address\n");
        return 0;
    }
    int i;
    printf("%p ",ptr);
    for(i=0;i<15;i++){
        ptr = see_one(ptr);
    }
    printf("\n");
    dstack_push(ptr);
}

int cmd_load(){
    return src_file("test.ff");
}

int cmd_mkdir(){
     U32 cnt; char* ptr = src_word(&cnt);
     //careful, no pathing
    HINDEX h = head_new(0,H_DIR,T_NA,search_list[0]);
    head_append_source(h,ptr,cnt);
    head_commit(h);

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
            if(0==strncmp(ptr,"cd",2)) { cmd_cd(); return 1;  };
            if(0==strncmp(ptr,"ok",2)) { return 1;  };
            break;
        case 3:
            if(0==strncmp(ptr,"pwd",3)) { printf("%s\n",head_get_name(search_list[0])); return 1;}
            if(0==strncmp(ptr,"run",3)) { call_meow(var->run_ptr); return 1;}
            if(0==strncmp(ptr,"see",3)) {return cmd_see();}
            if(0==strncmp(ptr,"sys",3)) { return cmd_sys();}
            break;
        case 4:
            if(0==strncmp(ptr,"exit",4)) {exit(0);}
            if(0==strncmp(ptr,"save",4)) {return cmd_save();}
            if(0==strncmp(ptr,"list",4)) {return cmd_list();}
            if(0==strncmp(ptr,"load",4)) {return cmd_load();}
            break;
        case 5:
            if(0==strncmp(ptr,"mkdir",5)) {return cmd_mkdir();}
            break;
    }
    return 0;
}

