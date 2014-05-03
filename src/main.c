#include  <stdio.h>// printf
#include <stdlib.h>

#include <sys/mman.h>

#include "global.h"
#include "header.h"
#include "data.h"
#include "src.h"
#include "lang.h"

sVar*   var;                    //system variables

#define        DSP_SIZE 1024
#define        RSP_SIZE 1024
#define TABLE_SIZE (CODE_SIZE/16)

/*
 * Attach code to a header...
*/
HINDEX H_ROOT;
HINDEX H_PROC;
HINDEX H_DIR;
HINDEX H_U32;


void head_build(){
//                         name     pcode,type  PARM      DAD
         H_ROOT = head_new("",0,        0,0,      T_NA,         0); //type is DIR           
  HINDEX H_TYPE = head_new("TYPE",4,    0,0,      T_NA,      H_ROOT);
  
         H_DIR =  head_new("DIR",3,     0,H_TYPE, T_NA,      H_TYPE); //dad is TYPE  
         H_PROC = head_new("PROC",4,    0,H_TYPE, T_NA,      H_TYPE);
         H_U32 = head_new("U32",3,      0,H_TYPE, T_NA,      H_TYPE);
//TODO: fixup type (DIR) for root and type...!
printf("ROOT IS %p\n",H_ROOT);
head_dump_one(H_ROOT);
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
    // Read parameter code
    PARM parm;
    fread(&parm,1,1,f);
    // skip 3 bytes
    char notused[3];
    fread(&notused,3,1,f);
 // Now read code into the data section...
  U32 datalen;
  fread(&datalen,4,1,f);                        //read data length
  data_align4_minus_1();
  data_compile_U8(0);                           //code token
  U8* data = data_compile_from_file(f,datalen);
  // And update the head
  head_set_type(h,H_PROC);        //it was created as DIR originally...
  head_set_parm(h,parm);          //from file...
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
printf("kernel_load 1\n");
  while(kernel_load_record(f)) {};
  fclose(f);
printf("kernel_load done\n");
}




int main(int argc, char **argv)
{
printf("\33[0;40m");	
//---------------------------------------------------------------------
// Data segment.  Houses var at the bottom...  
//
        // memmap data segment
        U8* data_base = mmap((void*)0x04000000,
                 CODE_SIZE,
                 PROT_READ+PROT_WRITE+PROT_EXEC,
                 MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED,
                 0,0);
        //var structure is placed into TABLE!
        var = (sVar*)data_base;
        var->data_ptr = data_base + sizeof(sVar);

        // install system var structure at bottom
        var->data_base = data_base;
        var->data_top = data_base + CODE_SIZE;

        printf("data at %p->%p ",var->data_base,var->data_top);

//---------------------------------------------------------------------
// Table - runtime 
//
        U8*table_base = mmap((U8**)(CODE_ADDRESS/4),
                 sizeof(TINDEX)*TABLE_SIZE,
                 PROT_READ+PROT_WRITE+PROT_EXEC,
                 MAP_ANONYMOUS|MAP_SHARED|MAP_FIXED,
                 0,0);
        var->table_base = table_base;
        printf("TABLE at %p ",var->table_base);
        var->table_top = (U8*)var->table_base + sizeof(TINDEX)*TABLE_SIZE;
       // var->table_ptr = (U8**)var->table_base;
       // *var->table_ptr++ = 0 ; //first table entry is always 0 !
      
//---------------------------------------------------------------------
// DSP
//
// 
        var->dsp_base = (U8*)malloc(DSP_SIZE);
        var->dsp_top = var->dsp_base + DSP_SIZE;
 printf("DSP at %p ",var->dsp_top);
//---------------------------------------------------------------------
// RSP
        var->rsp_base = (U8*)malloc(RSP_SIZE);
        var->rsp_top = var->rsp_base + RSP_SIZE;
        var->sp_meow = (sRegsMM*)var->rsp_top;
 printf("RSP at %p ",var->rsp_top);
//---------------------------------------------------------------------
// HEAD
        var->head_base = (U8*)malloc(HEAD_SIZE);
        var->head_top = var->head_base + HEAD_SIZE;
        var->head_ptr = var->head_base;
 printf("HEAD at %p \n",var->head_base);
//---------------------------------------------------------------------
     
        head_build();
  printf("data pointer is now at %p\n",var->data_ptr);
        kernel_load();
  printf("data pointer is now at %p\n",var->data_ptr);
  
        cmd_init();
        lang_init();

//        int i;
//for(i=0;i<20;i++)

      
// U32 qqq = xxx(0x3456,0x1234);
// printf("bindings returns %x\n",qqq);
    src_init();
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