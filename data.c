#include "global.h"
extern sVar* var;
void data_align4(void){
    while(0x3 & (U32)var->data_ptr)
        *var->data_ptr++ = 0xEE;
}
/*=============================================================================
  align at 4-bound -1, to insert a token before aligned code, for instance...
*/
void data_align4_minus_1(void){
    while(3 != (0x3 & (U32)var->data_ptr))
        *var->data_ptr++ = 0xEE;
}


U8* data_compile_U8(U8 val){
  U8* ret = var->data_ptr;
  *var->data_ptr++ = val;
  return ret;
}
U8* data_compile_U16(U16 val){
  U16* ret = var->data_ptr;
  *ret++ = val;
  var->data_ptr+=2;
  return (U8*)ret;
}

U8* data_compile_U32(U32 val){
  U32* p = (U32*)var->data_ptr;
  *p=val;
  var->data_ptr+=4;
  return (U8*) p;
}

U8* data_compile_blob(U8*blob,U32 cnt){
  U8* ret = var->data_ptr;
//printf("compile blob from %x %d to %x\n",blob,cnt,ret);
  while(cnt--)
    *var->data_ptr++ = *blob++;
  return ret;
}

U8* data_compile_from_file(FILE* f,U32 cnt){
  U8* ret = var->data_ptr;
//printf("data_compile_from_file: %d bytes at %x\n",cnt,ret);
  if(cnt != fread(ret,1,cnt,f))
    printf("ERROR: data_compile_from_file could not read %x bytes of code\n",cnt);
  var->data_ptr += cnt;
  
  return ret;
}