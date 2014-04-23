#include "global.h"
extern sVar* var;

TINDEX table_add_ptr(U8* ptr){
  
  *var->table_ptr = ptr;
  var->table_ptr++;
  U32 ret = (var->table_ptr - var->table_base);
  return ret;
}