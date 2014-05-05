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
#include <string.h>
#include "global.h"
#include "table.h"
#include "header.h"

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
   compile_token, given a target address... Any target will work...
*/    
int data_compile_token_p(U8* target){
    U8**tbase = table_base(var->data_ptr);
    TOKEN tok = table_find_or_create(var->data_ptr,target);
    if(tok){
        *var->data_ptr++ = tok; //compile token
        return 1;
    }
    // There was not a single empty slot in the reachable part of the table...
    printf("data_compile_token: ERROR - no empty space...\n");
    return 0;
/* ============================================================================
   compile_token, given a head
*/    
}
int data_compile_token(HINDEX h){
//printf("interpret_comp: %d %s \n",h,head_get_name(h))
    //get table base for this location+1
    //+4 since index 0 is used for 'code'
//    U8**tbase = (U8**)(((U32)(var->data_ptr+1) >>2) & 0xFFFFFFFC);
//printf("data_compile_token to: %08X, base %08X\n",var->data_ptr+1,tbase);
    //now, in the range of 1-255, try to find the entry represented by
    //HINDEX h...  
    U8* target = head_get_code(h); //that's what HINDEX h targets...
    return data_compile_token_p(target);
}
/* ============================================================================
 * save
 * 
 * Data is pure, and has no pointers.  Except system variables at the bottom...
 */
int data_save(FILE* f){
    U32 size = var->data_ptr-var->data_base;
    if(1 != fwrite(var->data_base,size,1,f))
        return 0;
    return 1;
}
int data_load(FILE* f){
    //load the sVar to see
    sVar fvar;
  //  ret = fread(&fvar,sizeof(sVar),1,f);
  //  if(1!= ret) return 0;
    //verify base
    
    U32 size = var->data_ptr-var->data_base;
    if(1 != fwrite(var->data_base,size,1,f))
        return 0;
    return 1;
}
    
int data_ref_style_p(HINDEX htarget,HINDEX hoperation){
    data_compile_token(hoperation);
    data_compile_token(htarget);
    return 1;
}
int data_ref_style(HINDEX htarget,char* abspath){
    U32 len = strlen(abspath);
    if(!htarget) {
        return 0; //TODO: error
    }
    HINDEX href = head_find_absolute(abspath,len); //
    if(!href) {
        printf("data_ref_style: can't find [%s]\n",abspath);
        return 0;
    }
    return data_ref_style_p(htarget,href);
}
