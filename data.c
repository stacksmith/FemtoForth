#include "global.h"
extern U8* data_base;
extern U8* data_ptr;

U8* data_compile_U8(U8 val){
  U8* ret = data_ptr;
  *data_ptr++ = val;
  return ret;
}

U8* data_compile_U32(U32 val){
  U32* p = (U32*)data_ptr;
  *p=val;
  data_ptr+=4;
  return (U8*) p;
}

U8* data_compile_blob(U8*blob,U32 cnt){
  U8* ret = data_ptr;
printf("compile blob from %x %d to %x\n",blob,cnt,ret);
  while(cnt--)
    *data_ptr++ = *blob++;
  return ret;
}

U8* data_compile_from_file(FILE* f,U32 cnt){
  U8* ret = data_ptr;
  if(cnt != fread(data_ptr,1,cnt,f))
    printf("ERROR: data_compile_from_file could not read %x bytes of code\n",cnt);
  data_ptr += cnt;
  return ret;
}