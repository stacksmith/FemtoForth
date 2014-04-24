//
// Source
//
#include "global.h"

char src_buf[256]="";
char* src_ptr = src_buf;
char* src_saveptr=0;


void src_init(){
}
void src_line(){
    gets(src_buf);
    src_ptr = src_buf;
}
void src_ws(){
  while(1){
    char c = *src_ptr++;
    switch(c){
      case ' ': break;
      case '\t': break;
      case 0x0D: break;
      case 0x0A: break;
      case 0: src_line(); break;
      default: src_ptr--; return;
    }
  }
}

U32 src_cnt(){
  U32 cnt=0;
  char*p = src_ptr;
  while(1){
    char c = *p++;
    switch(c){
      case ' ': return cnt;
      case '\t': return cnt;
      case 0x0D: return cnt;
      case 0x0A: return cnt;
      case 0: return cnt;
      default: 
        cnt++;
    }
  }
}

U32 src_one(){
  src_ws();
  return src_cnt();
}
