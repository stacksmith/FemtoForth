/******************************************************************************
Copyright 2014 Victor Yurkovsky

This file is part of the FemtoForth project.

FemtoForth is free software: you can redistribute it and/or modify
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
sMemLayout* lay;

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
    search_list[0] = H_TYPE;   //wd
    search_list[1] = head_find_abs_or_die("system'core");    //
    search_list[2] = head_find_abs_or_die("system'io");      //
    search_list[3] = head_find_abs_or_die("system");    //
    search_list[4] = H_ROOT;    //
   //
    var->wd = head_find_abs_or_die("test"); 
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
    color(COLOR_RESET); color(FORE_WHITE);
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
  // handle cd .. as one up
  if(  ((cnt==2)&&(*ptr=='.')&&(*(ptr+1)=='.') )){
    x = head_get_dad(var->wd);
  } else  
    // all non '... entries are relative searches
    if( ! ((cnt==1)&&(*ptr=='\'')) )
        x = head_find(ptr,cnt,search_list);
  // handle cd .. as cd up
  if(x) {
      var->wd = x;
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
  save 
  
  save the current system as 3 segments: data,table and head.
  
 ==========================================================================*/
U32 cmd_img_save_p(FILE* f){
    size_t ret, required, total;
    total = 0;
    //TODO: write an ID header...
    //---------------------------------------------
    //first write the data, so that vars are first
    required = var->data_ptr - lay->data_bottom;
    total += ret = fwrite(lay->data_bottom,1,required,f);
    if(required != ret) return 0;
printf("cmd_img_save_p wrote %d, total %d\n",ret,total);
    //---------------------------------------------
    //now write the table 
    //for stability, write the entire reachable portion
    U8* table_end = ((U8*)table_base(var->data_ptr)) + 255;
    required = table_end - lay->table_bottom;
    total += ret = fwrite(lay->table_bottom, 1,required,f);
    if(required != ret) return 0;
printf("cmd_img_save_p wrote %d, total %d\n",ret,total);
    
    //now write the headers
    required = var->head_ptr - lay->head_bottom;
    total += ret = fwrite(lay->head_bottom,1,required,f);
    if(required != ret) return 0;
printf("cmd_img_save_p wrote %d, total %d\n",ret,total);
    
    return total;
}

int cmd_img_save(){
    char* fname = "femto.img";
    FILE* f = fopen(fname,"w");
    if(!f) return 0;
    U32 ret = cmd_img_save_p(f);
    fclose(f);
    printf("img_save: wrote %d bytes to file %s\n",ret,fname);
    return 1;
}

/*=============================================================================
  load 
  
  load the current system as 3 segments: data,table and head.
  
 ==========================================================================*/
U32 cmd_img_load_p(FILE* f){
    size_t ret, required, total=0;
     //TODO: read an ID header...
    //---------------------------------------------
    //first read the header to determine the rest
    sMemLayout* newlay = (sMemLayout*)malloc(HOST_RESERVED);
    required = HOST_RESERVED;
    total += ret = fread(newlay,1,required,f);
    if(required != ret) return 0;
printf("cmd_img_load_p read %d, total %d\n",ret,total);
    sVar* newvar = (sVar*)malloc(SYS_RESERVED);
    required = SYS_RESERVED;
    total += ret = fread(newvar,1,required,f);
    if(required != ret) return 0;
printf("cmd_img_load_p read %d, total %d\n",ret,total);
    //---------------------------------------------
    // check that the positions are adequate
    if( (lay->data_bottom != newlay->data_bottom) ||
        (lay->table_bottom != newlay->table_bottom) ||
        (lay->head_bottom != newlay->head_bottom) ) {
        printf("cmd_img_load_p: cannot load to a different address yet\n");
    }
    //for now, just load segments.
    //---------------------------------------------
    // rest of data
    required = newvar->data_ptr - newlay->data_bottom - (HOST_RESERVED + SYS_RESERVED);
    U8* dest = lay->data_bottom + HOST_RESERVED + SYS_RESERVED;
    total += ret = fread(dest,1,required,f);
printf("cmd_img_load_p DATA: read %d, required %d, total %d\n",ret,required,total);
    if(required != ret) return 0;
    //---------------------------------------------
    // tabe
    U8* table_end = ((U8*)table_base(newvar->data_ptr)) + 255;
    required = table_end - newlay->table_bottom;
    dest = lay->table_bottom;
    total += ret = fread(dest,1,required,f);
printf("cmd_img_load_p TABE: read %d, required %d, total %d\n",ret,required,total);
    if(required != ret) return 0;
    //---------------------------------------------
    // head
    required = newvar->head_ptr - newlay->head_bottom;
    dest = lay->head_bottom;
    total += ret = fread(dest,1,required,f);
printf("cmd_img_load_p HEAD: read %d, required %d, total %d\n",ret,required,total);
    if(required != ret) return 0;
    //---------------------------------------------
    // Now, adjust the pointers
    interpret_init();   //set up registers and sp_meow...
    //
    var->data_ptr       = newvar->run_ptr;
    var->run_ptr        = newvar->run_ptr;
    var->run_table      = newvar->run_table;    //should eliminate
    var->head_ptr       = newvar->head_ptr;
    var->wd             = newvar->wd;
    //initialize source
    src_reset();        //this takes care of srcptr and more...
    
    return total;
}
int cmd_img_load(){
    char* fname = "femto.img";
    FILE* f = fopen(fname,"r");
    if(!f) return 0;
    U32 ret = cmd_img_load_p(f);
    fclose(f);
    printf("img_load: read %d bytes from file %s\n",ret,fname);
    return 1;
}

/*
TOKEN* see_one(TOKEN*p){
    PTOKEN* base = table_bottom(p);
    U8 token = *p;
    //return?
    if(!token){
        color(COLOR_DIM);  color(FORE_WHITE);printf("<");
        color(COLOR_RESET);color(FORE_GREEN);printf(";");
        color(COLOR_DIM); color(FORE_WHITE); printf(">");
        color(COLOR_RESET);
        return(p++);
    }

    color(COLOR_DIM);color(FORE_WHITE);printf("%p ",p);
    
    //print token
    PTOKEN target = base[*p];
    U32 off;
    HINDEX head = head_resolve(target,&off);
//printf("see_one %p [%s]\n",head,head_get_name(head));
    //dim the brackets; token is gree
    color(COLOR_DIM);color(FORE_WHITE);printf("<");color(COLOR_RESET); color(FORE_GREEN);    
    printf("%.*s",head_get_namelen(head),head_get_name(head));
    color(COLOR_DIM);color(FORE_WHITE);printf(">");color(COLOR_RESET);     
    p++;
    HINDEX type=head_get_type(head);

    if(T != type){
        color(COLOR_DIM);  color(FORE_WHITE);printf("<");
        color(COLOR_RESET); color(FORE_YELLOW);
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
            case T_REF: // TODO:*** 
                target = base[*p];
                HINDEX head = head_resolve(target,&off);
                printf("%.*s",head_get_namelen(head),head_get_name(head) );
                p++;
                break;
            default:
                printf("UNIMPLEMENTED type in decompiler %d\n",type);
                color(COLOR_RESET); color(FORE_WHITE);
                exit(0);
        }
        color(COLOR_DIM);  color(FORE_WHITE);printf(">");
    }
    color(COLOR_RESET); color(FORE_WHITE);
    
    printf("\n");
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
    if ((ptr < lay->data_bottom) || (ptr > lay->data_top)){
        src_error("see: illegal address\n");
        return 0;
    }
    int i;
    for(i=0;i<15;i++){
        if(ptr >= var->data_ptr)
            break;
        ptr = see_one(ptr);
    }
    dstack_push(ptr);
    color(COLOR_RESET);
}
*/
int cmd_load(){
    U32 cnt; char* ptr = src_word(&cnt);          //next word
    
    return src_file(ptr,cnt);
}

int cmd_mkdir(){
     U32 cnt; char* ptr = src_word(&cnt);
     //careful, no pathing
    HINDEX h = head_new(0,H_DIR,var->wd);
    head_append_source(h,ptr,cnt);
    head_commit(h);

}
/*int cmd_anus(){
    HINDEX h = head_get_root();
   while(h < (HINDEX)var->head_ptr){
printf("anus: h is %p\n",h);
       head_dump_one(h);
        h = (HINDEX)(head_size(h) + (U32)h);
    }
}
*/
//TODO: check error conditions, return...
int command(char* ptr,U32 cnt){
    switch(cnt){
        case 2:
            if(0==strncmp(ptr,"ls",2)) { cmd_ls(var->wd);return 1; }
            if(0==strncmp(ptr,"cd",2)) { cmd_cd(); return 1;  };
            if(0==strncmp(ptr,"ok",2)) { return 1;  };
            break;
        case 3:
            if(0==strncmp(ptr,"pwd",3)) { printf("%s\n",head_get_name(var->wd)); return 1;}
            if(0==strncmp(ptr,"run",3)) { call_meow(var->run_ptr); return 1;}
//            if(0==strncmp(ptr,"see",3)) {return cmd_see();}
            if(0==strncmp(ptr,"sys",3)) { return cmd_sys();}
            break;
        case 4:
            if(0==strncmp(ptr,"exit",4)) {exit(0);}
            if(0==strncmp(ptr,"list",4)) {return cmd_list();}
            if(0==strncmp(ptr,"load",4)) {return cmd_load();}
            break;
        case 5:
            if(0==strncmp(ptr,"mkdir",5)) {return cmd_mkdir();}
            if(0==strncmp(ptr,"clean",5)) {
                return table_clean((PTOKEN*) dstack_pop());
            }
            break;
        case 8:
            if(0==strncmp(ptr,"img_save",8)) {return cmd_img_save();}
            if(0==strncmp(ptr,"img_load",8)) {return cmd_img_load();}
            break;
    }
    return 0;
}

