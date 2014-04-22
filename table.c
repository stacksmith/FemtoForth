#include "global.h"
extern U8** table_base;
extern U8** table_ptr;

TINDEX table_add_ptr(U8* ptr){
  *table_ptr++ = ptr;
  U32 ret = (table_ptr - table_base);
  return ret;
}