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
#include  <stdio.h>// printf
#include <stdlib.h>

#include <sys/mman.h>

#include "global.h"
#include "header.h"
#include "data.h"
#include "src.h"
#include "lang.h"
#include "color.h"

sVar*   var;                    //system variables
sMemLayout* lay;                //memory layout for THIS system

#define        DSP_SIZE 1024
#define        RSP_SIZE 1024
//entry count in table
#define TABLE_SIZE (CODE_SIZE/16)

/*
 * Attach code to a header...
*/
HINDEX H_ROOT;
HINDEX H_PROC;
HINDEX H_DIR;
HINDEX H_U32;
HINDEX H_TYPE;
HINDEX H_SYSVAR;
HINDEX H_SYSTEM;

void head_build(){
//                         code,        type    DAD
         H_ROOT = head_new(0,           0,      0); //type is DIR
  head_commit(H_ROOT);
         H_SYSTEM = head_new(0,         0,      H_ROOT);
  head_commit(head_append_source(H_SYSTEM,"system // system internals",0));
         H_TYPE = head_new(0,           0,      H_SYSTEM);
  head_commit(head_append_source(H_TYPE,"TYPE // contains types",0));
         H_DIR =  head_new(0,           0,      H_TYPE); //dad is TYPE 
  head_commit(head_append_source(H_DIR,"DIR ",0));       
         H_PROC = head_new(0,           H_TYPE, H_TYPE);
  head_commit(head_append_source(H_PROC,"PROC // procedure directory",0));
         H_U32 =  head_new(0,           H_TYPE, H_TYPE);
  head_commit(head_append_source(H_U32,"U32 // procedure directory",0));
      H_SYSVAR =  head_new(0,           H_TYPE, H_TYPE);
  head_commit(head_append_source(H_SYSVAR,"SYSVAR // procedure directory",0));
         //H_U32 = head_new("U32",3,      0,H_TYPE, T_NA,      H_TYPE);
  
  
  head_set_type(H_ROOT,H_DIR);  
  head_set_type(H_SYSTEM,H_DIR);
  head_set_type(H_TYPE,H_DIR);      
  head_set_type(H_DIR,H_TYPE);      

//  head_dump_one(H_ROOT);
  
    
//head_dump_one(H_ROOT);
}
#include "interpret.h"

//void build_kernel(char* name,
/*
 * load kernel.
 * - create names and directories as needed
 * - 
 */
int kernel_load_record(FILE* f){
//printf("kernel_load_record %d\n",1);
    // Read the byte-long name length... 0 terminates
    U8 namelen;
    fread(&namelen,1,1,f);
    if(!namelen) return 0;
    // Read name into a buffer...
    char buf[256];
    fread(buf,namelen,1,f);       //read string
//*** DEBUG - insert name in data so we can see dumps
//data_compile_blob(buf,strlen(buf));
//*** DEBUG END
    //create a header...
    HINDEX h = head_find_or_create(buf);          //create header
    // Read parameter code.  Now  used as type specifier.
    PARM parm;
    fread(&parm,1,1,f);
    // skip 3 bytes
    char notused[3];
    fread(&notused,3,1,f);
 // Now read code into the data section...
  U32 datalen;
  fread(&datalen,4,1,f);                        //read data length
  data_align4_minus_1();
  //keep track of datasize...
  U8* datastart = var->data_ptr;
  
  data_compile_U8(0);                           //code token
  U8* data = data_compile_from_file(f,datalen);
  head_set_datasize(h,var->data_ptr - datastart);
  // And update the head
  // set type based on param...
  HINDEX mytype= head_find_abs_or_die("system'TYPE'PROC");
  U32 paytype = 0;
  switch(parm){
      case T_PROC: break;
      case T_U8:   paytype = PAYLOAD_ONE; break;
      case T_U16:  paytype = PAYLOAD_TWO; break;
      case T_U32:  paytype = PAYLOAD_FOUR; break;
      case T_OFF:  paytype = PAYLOAD_OFF8; break;
      case T_STR8: paytype = PAYLOAD_STR8; break;
      case T_REF:  paytype = PAYLOAD_REF; break;
      case T_DIR:  mytype = head_find_abs_or_die("system'TYPE'DIR");break;
      default: printf("kernel_load_record: invalid type %d\n",parm);
        return 0;
  }   
  head_set_type(h,mytype);        //it was created as DIR originally...
  head_set_ptype(h,paytype);
  head_set_code(h,data-1);       //point at 0 (code) token
  
//interpret_ql(data);
    return 1;
}

void kernel_load(){
  FILE* f;
  f=fopen("kernel.bin","r");
  if(!f) {
    printf("kernel_load:file is %p\n",f);
    exit(0);
  }
//printf("kernel_load 1\n");
  while(kernel_load_record(f)) {
   //   fprintf(stderr,".");
  };
  
  fclose(f);
//printf("kernel_load done\n");
}




int main(int argc, char **argv)
{
printf("\33[0;40;33m\n");	
printf("    FemtoForth  Copyright (C) 2014 Victor Yurkovsky\n");
printf("    This program comes with ABSOLUTELY NO WARRANTY'.\n");
printf("    This is free software, and you are welcome to redistribute it\n");
printf("    under certain conditions; type 'license' for details.\n\n");
color(COLOR_RESET);color(FORE_WHITE);
//---------------------------------------------------------------------
// Data segment.  Houses var at the bottom...  
//
        // memmap data segment
        U8* data_base = mmap((void*)0x04000000,
                 CODE_SIZE,
                 PROT_READ+PROT_WRITE+PROT_EXEC,
                 MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED,
                 0,0);
        //var structure is placed into bottom RESERVED (512) bytes of data!
        lay = (sMemLayout*)data_base;           // HOST_RESERVED bytes 
        var = (sVar*)(data_base+HOST_RESERVED); // SYS_RESERVED bytes 
        
        // install system var structure at bottom
        lay->data_bottom = data_base;
        lay->data_top = data_base + CODE_SIZE;
        //
        var->data_ptr = lay->data_bottom + HOST_RESERVED + SYS_RESERVED;

        printf("data at %p->%p ",lay->data_bottom,lay->data_top);

//---------------------------------------------------------------------
// Table - runtime 
//
        lay->table_bottom = mmap((U8**)(CODE_ADDRESS/4),
                 sizeof(TINDEX)*TABLE_SIZE,
                 PROT_READ+PROT_WRITE+PROT_EXEC,
                 MAP_ANONYMOUS|MAP_SHARED|MAP_FIXED,
                 0,0);
        printf("TABLE at %p ",lay->table_bottom);
        lay->table_top = (U8*)lay->table_bottom + sizeof(TINDEX)*TABLE_SIZE;
       // var->table_ptr = (U8**)var->table_bottom;
       // *var->table_ptr++ = 0 ; //first table entry is always 0 !
      
//---------------------------------------------------------------------
// DSP
//
// 
        lay->dsp_bottom = (U8*)malloc(DSP_SIZE);
        lay->dsp_top = lay->dsp_bottom + DSP_SIZE;
 printf("DSP at %p ",lay->dsp_top);
//---------------------------------------------------------------------
// RSP
        lay->rsp_bottom = (U8*)malloc(RSP_SIZE);
        lay->rsp_top = lay->rsp_bottom + RSP_SIZE;
        var->sp_meow = (sRegsMM*)lay->rsp_top;
 printf("RSP at %p ",lay->rsp_top);
//---------------------------------------------------------------------
// HEAD
        lay->head_bottom = (U8*)malloc(HEAD_SIZE);
        lay->head_top = lay->head_bottom + HEAD_SIZE;
        var->head_ptr = lay->head_bottom;
 printf("HEAD at %p \n",lay->head_bottom);
//---------------------------------------------------------------------
// SRC 
        lay->src_bottom =  (char*)malloc(256);
        src_reset(); 
      
        head_build();
     
//  printf("data pointer is now at %p\n",var->data_ptr);
        kernel_load();
//  printf("data pointer is now at %p\n",var->data_ptr);
  
        cmd_init();
        lang_init();

//        int i;
//for(i=0;i<20;i++)

      
// U32 qqq = xxx(0x3456,0x1234);
// printf("bindings returns %x\n",1);
    interpret_init();
// src_error("ass");
//  call_meow();
  
 
   while(1)
      interpret_outer();
 // line();
//        int z = armFunction(99);
//        printf("assembly returnst %d\n",z);       
        exit(0);
}