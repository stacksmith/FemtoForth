#include  <stdio.h>// printf
#include <stdlib.h>

#include <sys/mman.h>

#include "global.h"
#include "header.h"
#include "data.h"
#include "src.h"

sVar*   var;                    //system variables

#define        DSP_SIZE 1024
#define        RSP_SIZE 1024
#define TABLE_SIZE (CODE_SIZE/16)

/*
 * Attach code to a header...
*/
HINDEX H_PROC;
HINDEX H_DIR;

void head_build(){
  head_new("",0, 0,0,0,0);
  HINDEX H_ROOT =       head_new("",0,          0,2,      T_NA,         0); //type is DIR           
  HINDEX H_TYPE =       head_new("TYPE",4,      0,3,      T_NA,         H_ROOT);
         H_DIR =        head_new("DIR",3,       0,H_TYPE, T_NA,         H_TYPE); //dad is TYPE  
         H_PROC =       head_new("PROC",4,      0,H_TYPE, T_NA,         H_TYPE);
//  HINDEX H_IO =         head_new("io",2,        0,H_DIR,  T_NA,         H_ROOT);
  
  
/*  HINDEX H_EMIT =       head_new("emit",        0,H_PROC, T_NONE,       H_IO);
   head_code(H_EMIT,(U8*)emit,(U8*)emit_x);
  HINDEX H_KEY =        head_new("key",         0,H_PROC, T_NONE,       H_IO);
   head_code(H_KEY,(U8*)key,(U8*)key_x);
  HINDEX H_TTT =        head_new("ttt",         0,H_PROC, T_NONE,       H_IO);
   head_code(H_TTT,(U8*)ttt,(U8*)ttt_x);
*/  
}
#include "interpret.h"

//void build_kernel(char* name,
/*
 * load kernel.
 * - create names and directories as needed
 * - 
 */
void kernel_load_record(U32 namelen,FILE* f){
  char buf[256];
  fread(buf,namelen,1,f);       //read string
//*** DEBUG - insert name in data so we can see dumps
//data_compile_blob(buf,strlen(buf));
//*** DEBUG END
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

}

void kernel_load(){
  FILE* f;
  f=fopen("/data/tmp/kernel.bin","r");
printf("kernel_load:file is %x\n",f);
  while(1){
    U8 namelen;
    fread(&namelen,1,1,f);
    if(!namelen) break;
    kernel_load_record(namelen,f);
  }
  fclose(f);
printf("kernel_load done\n");
}




int main(int argc, char **argv)
{
        printf("Hello.  size of header is %d\n",sizeof(sHeader));
	
//---------------------------------------------------------------------
// Table - runtime 
//
        U8*table_base = mmap((U8**)(CODE_ADDRESS/4),
                 sizeof(TINDEX)*TABLE_SIZE,
                 PROT_READ+PROT_WRITE+PROT_EXEC,
                 MAP_ANONYMOUS|MAP_SHARED|MAP_FIXED,
                 0,0);
        //var structure is placed into TABLE!
        var = (sVar*)table_base;
        var->table_base = table_base;
        printf("TABLE at %p ",var->table_base);
        var->table_top = (U8*)var->table_base + sizeof(TINDEX)*TABLE_SIZE;
       // var->table_ptr = (U8**)var->table_base;
       // *var->table_ptr++ = 0 ; //first table entry is always 0 !
 //---------------------------------------------------------------------
// data -  
//
        // memmap data segment
        U8* data_base = mmap((void*)0x04000000,
                 CODE_SIZE,
                 PROT_READ+PROT_WRITE+PROT_EXEC,
                 MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED,
                 0,0);
        // install system var structure at bottom
        var->data_base = data_base;
        var->data_top = data_base + CODE_SIZE;
        var->data_ptr = data_base;

        printf("data at %p->%p ",var->data_base,var->data_top);
      
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
        void* phead = (sHeader*)malloc(HEAD_MAX*sizeof(sHeader));
        head_set_segment(phead);
 printf("HEAD at %p \n",phead);
//---------------------------------------------------------------------
     
        head_build();
  printf("data pointer is now at %08p\n",var->data_ptr);
        kernel_load();
  printf("data pointer is now at %08p\n",var->data_ptr);
  
        cmd_init();
    int i;
for(i=0;i<20;i++)
  head_dump_one(i);

      
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