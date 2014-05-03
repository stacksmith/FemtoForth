//
// Source
//
#include <string.h>
#include "global.h"

#define SRC_BUF_SIZE 256
//char* src_buf;
extern sVar* var;
//char* src_ptr;
FILE* hfile;  
U32 lineno;

typedef struct sSrcContext{
    
}sSrcContext;


void src_reset(){
    if(hfile != stdin) fclose(hfile);
    hfile = stdin;                   //initial
    *var->src_base=0;                         //will trigger initial src_line!
     var->src_ptr = var->src_base;
    lineno = 0;
}

void src_init(){
    var->src_base = (char*)malloc(256);
    hfile = stdin;
    src_reset();
//printf("SOURCE RESET\n");
}

void src_skip_line(){
    char c;
    while(1){
        switch(c=*var->src_ptr){
            case '\r':
            case '\n':
            case 0:
                return;
            default:
                var->src_ptr++;
        }
    }
}

/*=============================================================================
 * SOURCE via file system....
 * ==========================================================================*/

//TODO: add file support
char* src_line(){
    if(NULL == fgets(var->src_base,SRC_BUF_SIZE,hfile)){
        src_reset();
        //printf("src_line: EOF\n");
    }
    var->src_ptr = var->src_base;
    lineno++;
    return var->src_base;
    
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
    char c = *var->src_ptr++;
//printf("src_ws %p\n",var->src_ptr);
//printf("src_ws [%c] %d pointer %p\n",c,c,var->src_ptr);
    if(0==c) {
        src_line();        //reload as necessary
    }
    else
        if(!src_is_ws(c)) {
            var->src_ptr--;
            return;
        }
  }
}

/*=============================================================================
 * cnt
 * 
 * Count the word at var->src_ptr, using ws
 * ==========================================================================*/
U32 src_cnt(){
  U32 cnt=0;
  char*p = var->src_ptr;
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
//printf("src_one %p\n",var->src_ptr);
  src_ws();
  return src_cnt();
}

char* src_word(U32* pcnt){
    *pcnt = src_one();          //parse a word, count
    char* ret = var->src_ptr;
    var->src_ptr += *pcnt;
    return ret;
}
/*=============================================================================
 * src_error_print
 * 
 * on error, print error line and position...
 * ==========================================================================*/
void src_error(char* msg){
    printf("\33[0;31mERROR %s\33[0;37m (%d)\n",msg,lineno);
    int i = var->src_ptr - var->src_base; //how far into the file are we in?
    if(i>=0){
        printf("%s",var->src_base);         //print entire line
        printf("\33[1;31m");
        while(i-- >0 ) printf("_");     //print error point
        printf("|\33[0;37m\n");
        src_reset();
    }
}
/*=============================================================================
 * src_file
 * 
 * redirect input to a file
 * TODO:reentrancy for includes
 * ==========================================================================*/
int src_file(char* fname){
    hfile = fopen(fname,"r");
    if(!hfile) {
        src_error("File error\n");
        return 0;
    }
    return 1;
}
