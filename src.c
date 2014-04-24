//
// Source
//
#include "global.h"

char* src_buf;
char* src_ptr;
char* src_saveptr=0;


void src_init(){
    src_buf = (char*)malloc(256);
    *src_buf=0;                         //will trigger initial src_line!
    src_ptr = src_buf;
    
}
//TODO: add file support
void src_line(){
    gets(src_buf);
    src_ptr = src_buf;
}
/*=============================================================================
 * is_ws
 * 
 * return 1 if the character is a terminating ws
 * ==========================================================================*/
int src_is_ws(char c){
    switch(c) {
      case ' ': 
      case '\t': 
      case 0x0D: 
      case 0x0A: 
          return 1;
    }
    return 0;
}

/*=============================================================================
 * ws
 * 
 * skip ws, reloading as necessary
 * ==========================================================================*/
void src_ws(){
  while(1){
    char c = *src_ptr++;
    if(!src_is_ws(c)) {
        src_ptr--;
        return;
    }
    if(0==c) src_line();        //reload as necessary
  }
}

/*=============================================================================
 * cnt
 * 
 * Count the word at src_ptr, using ws
 * ==========================================================================*/
U32 src_cnt(){
  U32 cnt=0;
  char*p = src_ptr;
  while(1){
    char c = *p++;
    if( (0==c) || src_is_ws(c))  return cnt;
    cnt++;
  }
}

U32 src_one(){
  src_ws();
  return src_cnt();
}
