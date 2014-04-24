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
//printf("src_ws [%c] %d\n",c,c);
    if(0==c) 
        src_line();        //reload as necessary
    else
        if(!src_is_ws(c)) {
            src_ptr--;
            return;
        }
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
/*=============================================================================
 * src_one
 * 
 * skip ws; return cnt...
 * ==========================================================================*/

U32 src_one(){
  src_ws();
  return src_cnt();
}
/*=============================================================================
 * src_error_print
 * 
 * on error, print error line and position...
 * ==========================================================================*/
void src_error(char* msg){
    printf("\33[0;31mERROR %s\33[0;37m\n",msg);
    int i = src_ptr - src_buf; //how far into the file are we in?
    if(i>=0){
        printf("%s\n",src_buf);         //print entire line
        while(i-- >0 ) printf("_");     //print error point
        printf("|\n");
    }
}