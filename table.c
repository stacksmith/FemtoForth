// Note: index 0 is not used (so not found = 0).  Index 1 is root
#include "global.h"
extern sVar* var;

TINDEX table_add_ptr(U8* ptr,HINDEX head){
  
  *var->table_ptr = ptr;
  U32 ret = (var->table_ptr - (U8**)var->table_base); //will return table index
  var->table_ptr++;
  
  return ret;
}

