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
//
// Source
//
#include <string.h>
#include "global.h"

#define SRC_BUF_SIZE 256
//char* src_buf;
extern sVar* var;
sMemLayout* lay;
//char* src_ptr;
FILE* hfile;  
U32 lineno;
char* src_errbuf; //buffer starts here

typedef struct sSrcContext{
    
}sSrcContext;


void src_reset(){
    if(hfile && (hfile != stdin)) 
        fclose(hfile);
 
    hfile = stdin;                   //initial
     var->src_ptr = lay->src_base;
    *var->src_ptr=0;                         //will trigger initial src_line!
    src_errbuf = var->src_ptr;
    lineno = 0;
}


void src_set(char* buf){
    var->src_ptr = buf;
    src_errbuf = var->src_ptr;
    lineno = 0; //???    
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
    if(NULL == fgets(lay->src_base,SRC_BUF_SIZE,hfile)){
        src_reset();
        //printf("src_line: EOF\n");
    }
    var->src_ptr = lay->src_base;
    lineno++;
    src_errbuf = var->src_ptr; //for error reporting
    return lay->src_base;
    
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
char* src_ws_p(char* p){
  while(1) {
    char c = *p++;
    if(!c || src_is_ws(c)) return p;
  }
}
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
U32 src_cnt_p(char*p){
  U32 cnt=0;
  while(1){
    char c = *p++;
    if( (0==c) || src_is_ws(c))  return cnt;
    cnt++;
  }
}
U32 src_cnt(){
  return src_cnt_p(var->src_ptr);
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
    int i = var->src_ptr - src_errbuf; //how far into the file are we in?
    printf("\33[2;31m");
    if(i>=0){
        char*p = src_errbuf;
        while(i){ putchar(*p++); i--;}
        
/*           
        printf("%s",src_errbuf);         //print entire line
        printf("\33[1;31m");
        while(i-- >0 ) printf("_");     //print error point
        printf("|\33[0;37m\n");
*/
    }
    src_reset();
    printf("\33[0m");
  
}
/*=============================================================================
 * src_file
 * 
 * redirect input to a file
 * TODO:reentrancy for includes
 * ==========================================================================*/
int src_file(char* fname,U32 cnt){
    char*p = strndup(fname,cnt);
    hfile = fopen(p,"r");
    free(p);
    if(!hfile) {
        printf("File error trying to open [%s]\n",p);
        src_error(" ");
        return 0;
    }
    return 1;
}
