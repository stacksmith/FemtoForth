#include "global.h"
#include "header.h"
extern sHeader*       HEAD;
// interpret.c
char src_buf[256]="";
char* src_ptr = src_buf;
char* src_saveptr=0;

/* TODO: fix this, pls */
typedef struct sInterp {
  HINDEX list[16];
}sInterp;

sInterp icontext = {4,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};




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
interpret_ls(HINDEX dir){
printf("ls in %d\n",dir);
  HINDEX h = HEAD[dir].child;
  while(h){
    printf("%s\n",HEAD[h].name);
    h=HEAD[h].next;
  } 
}

int interpret_cd(){
  U32 cnt = src_one();
  char* ptr = src_ptr;
  src_ptr += cnt;
  HINDEX x=1; //root
  // handle cd '  as cd to root...
  if(  ((cnt==2)&&(*ptr=='.')&&(*(ptr+1)=='.') )){
    x = HEAD[icontext.list[0]].dad;
  } else  
  if( ! ((cnt==1)&&(*ptr=='\'')) )
     x = head_find(ptr,cnt,icontext.list);
  // handle cd .. as cd up
  if(x) {
    icontext.list[0]=x;
    return 1;
  } else{
    printf("ERROR: cd could not cd to [%s]\n",ptr);
    return 0;
  }
}

int interpret_one(){
  U32 cnt = src_one();
  char* ptr = src_ptr;
  src_ptr += cnt;
  
  if(0==strncmp(ptr,"ls",2)) { interpret_ls(icontext.list[0]);return 1; }
  if(0==strncmp(ptr,"cd",2)) { return interpret_cd();   };
  if(0==strncmp(ptr,"exit",4)) {exit(0);}
  if(0==strncmp(ptr,"pwd",3)) { printf("%s\n",HEAD[icontext.list[0]].name); return 1;}
   HINDEX x = head_find(ptr, cnt,icontext.list);
   if(x){
     printf("found %d\n",x);
     return 1;
   } else {
     printf("not found %s\n",src_ptr);     
     return 0;
   }
}

