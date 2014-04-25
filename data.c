#include "global.h"
#include "table.h"
#include "header.h"
extern sHeader*       HEAD;

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
  U16* ret = (U16*)var->data_ptr;
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
//TODO: error protect...
U8* data_compile_off_S8(U32 val){
  U8* ptr = var->data_ptr;
  int off = val - (1+(U32)ptr);
  if((off < -126) || (off > 127)) {
      printf("ERROR: data_compile_off_s8 RANGE PROBLEM: ptr %p target %08x\n",ptr,val);
  }
  *var->data_ptr++ = (off & 0xFF);
  return ptr;
}

U8* data_compile_from_file(FILE* f,U32 cnt){
  U8* ret = var->data_ptr;
//printf("data_compile_from_file: %d bytes at %x\n",cnt,ret);
  if(cnt != fread(ret,1,cnt,f))
    printf("ERROR: data_compile_from_file could not read %x bytes of code\n",cnt);
  var->data_ptr += cnt;
  
  return ret;
}
/* ============================================================================
 * compile_token...
 * 
 * 
 */

int data_compile_token(HINDEX h){
//printf("interpret_comp: %d %s \n",h,&HEAD[h].name);
    //get table base for this location+1
    //+4 since index 0 is used for 'code'
//    U8**tbase = (U8**)(((U32)(var->data_ptr+1) >>2) & 0xFFFFFFFC);
    U8**tbase = table_base(var->data_ptr);
//printf("data_compile_token to: %08X, base %08X\n",var->data_ptr+1,tbase);
    //now, in the range of 1-255, try to find the entry represented by
    //HINDEX h...  
    U8* target = HEAD[h].pcode; //that's what HINDEX h targets...
    TOKEN tok = table_find_or_create(var->data_ptr,target);
    if(tok){
        *var->data_ptr++ = tok; //compile token
        return 1;
    }
    // There was not a single empty slot in the reachable part of the table...
    printf("data_compile_token: ERROR - no empty space...\n");
    return 0;
    
}
