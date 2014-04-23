#include  <stdio.h>// printf
#include <stdlib.h>

#include <sys/mman.h>

#include "global.h"
#include "header.h"
#include "data.h"

U8*     data_base;
U8*     data_ptr;
U32*    dsp_base;

U8**    table_base;
U8**    table_ptr;
sHeader*       HEAD;
#define         DSP_SIZE 1024

/*
 * Attach code to a header...
*/
void head_code(HINDEX h,U8* start,U8* end){
  //copy code into data area...
  
  U8* pcode = data_compile_blob(start,end-start);
}
HINDEX H_PROC;
HINDEX H_DIR;

void head_build(){
  head_new("",0,0,0,0);
  HINDEX H_ROOT =       head_new("",            0,2,      T_NA,         0); //type is DIR           
  HINDEX H_TYPE =       head_new("TYPE",        0,3,      T_NA,         H_ROOT);
         H_DIR =        head_new("DIR",         0,H_TYPE, T_NA,         H_TYPE); //dad is TYPE  
         H_PROC =       head_new("PROC",        0,H_TYPE, T_NA,         H_TYPE);
  HINDEX H_IO =         head_new("io",          0,H_DIR,  T_NA,         H_ROOT);
  
      head_new("CRAP",        0,2,      T_NA,         H_TYPE);
  
/*  HINDEX H_EMIT =       head_new("emit",        0,H_PROC, T_NONE,       H_IO);
   head_code(H_EMIT,(U8*)emit,(U8*)emit_x);
  HINDEX H_KEY =        head_new("key",         0,H_PROC, T_NONE,       H_IO);
   head_code(H_KEY,(U8*)key,(U8*)key_x);
  HINDEX H_TTT =        head_new("ttt",         0,H_PROC, T_NONE,       H_IO);
   head_code(H_TTT,(U8*)ttt,(U8*)ttt_x);
*/  
}

//void build_kernel(char* name,

void kernel_load_record(U32 namelen,FILE* f){
  char buf[256];
  fread(buf,namelen,1,f);       //read string
  HINDEX h = head_find_or_create(buf);
  // Read parameter code
  PARM parm;
  fread(&parm,1,1,f);
  // skip 3 bytes
  char notused[3];
  fread(&notused,3,1,f);
  // Now read code into the data section...
  U32 datalen;
  fread(&datalen,4,1,f);                        //read data length
  U8* data = data_compile_from_file(f,datalen);
  // Add the pointer to the table
  TINDEX tin = table_add_ptr(data);
  // And update the head
  HEAD[h].index = tin;
  HEAD[h].type = H_PROC;        //it was created as DIR originally...
  HEAD[h].parm = parm;          //from file...

}

void kernel_load(){
  FILE* f;
  f=fopen("/data/tmp/kernel.bin","r");
printf("file is %d\n",f);
  while(1){
    U8 namelen;
    fread(&namelen,1,1,f);
    if(!namelen) break;
    kernel_load_record(namelen,f);
  }
  fclose(f);
}

#include "interpret.h"



int main(int argc, char **argv)
{
        printf("Hello.  size of header is %d\n",sizeof(sHeader));
	
        // memmap code segment
        data_base = mmap((void*)0x04000000,
                 CODE_SIZE,
                 PROT_READ+PROT_WRITE+PROT_EXEC,
                 MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED,
                 0,0);
        data_ptr = data_base;
        printf("data_base at %p ",data_base);
        //memmap table
	table_base = mmap((U8**)(CODE_ADDRESS/4),
		 CODE_SIZE/16*sizeof(void*),
		 PROT_READ+PROT_WRITE+PROT_EXEC,
		 MAP_ANONYMOUS|MAP_SHARED|MAP_FIXED,
		 0,0);
	printf("TABLE at %p ",table_base);
        table_ptr = table_base;
        //DSP
        dsp_base = (U32*)malloc(DSP_SIZE);
 printf("DSP at %p ",dsp_base);
        
        //
        HEAD = (sHeader*)malloc(HEAD_MAX*sizeof(sHeader));
 printf("HEAD at %p \n",HEAD);
      
        head_build();
        kernel_load();
        

int i;
for(i=0;i<10;i++)
  head_dump_one(i);
        interpret_ql(&HEAD[2]);
    while(1)
      interpret_one();
 // line();
//        int z = armFunction(99);
//        printf("assembly returnst %d\n",z);       
        exit(0);
}