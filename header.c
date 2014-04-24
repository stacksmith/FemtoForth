#include "global.h"
#include "header.h"
#include "data.h"
extern sHeader*       HEAD;
HINDEX hindex_last = 0;

HINDEX head_new(char* name,U32 cnt, U8*pcode,HINDEX type,PARM parm,HINDEX dad)
{
  HINDEX ret = hindex_last;
//printf("head_new: working on %d. (%s)\n",ret,name);
  sHeader* header = &HEAD[hindex_last++];   //claim a header
  header->dad = dad;
  header->child = 0;
  header->type = type;
  header->pcode = pcode;
  header->parm = parm;
  
  if(cnt>NAMELEN) cnt=NAMELEN; //truncate name...
  strncpy(header->name,name,cnt);
  if(ret){
    header->next = HEAD[dad].child;      //dad's first child is our sib
 //printf("head_new: %s's next is %d:%s\n",HEAD[ret].name,header->next,HEAD[header->next].name);
    HEAD[dad].child=ret;                //we are dad's first child
  } else {
    header->next = 0;
  }
  return ret;
}

HINDEX head_locate(HINDEX dir,char* name,U32 len){
  HINDEX h = HEAD[dir].child;
//printf("head_locate: dir is %d,child is %d[%s]\n",dir,h,HEAD[h].name);      
  while(h){
//printf("h=%d [%s]\n",h,HEAD[h].name);      
    if(0==strncmp(name,HEAD[h].name,len))
      return h;
if(HEAD[h].next==h){
  printf("ERROR: %s's next is itself!\n",HEAD[dir].name);
  exit(0);
}
    h = HEAD[h].next;
    
  }
  return h;
}
extern HINDEX H_PROC;
HINDEX head_find_or_create(char* path){
  HINDEX dir = 1;         //start at root
  char* name = strtok(path,"'");
  while(name){
    HINDEX found = head_locate(dir,name,strlen(name));
    if(!found) {
      dir = head_new(name,strlen(name),  0,H_PROC,T_NA,dir);
    } else {
      dir = found;
    }
    name = strtok(NULL,"'");
  }
  return dir;
}

HINDEX head_find_absolute(char* path,U32 len){
  HINDEX dir = 1;         //start at root
  char* name = strtok(path,"'");
  while(name){
    HINDEX found = head_locate(dir,name,strlen(name));
    if(!found)
      return 0;
    dir = found;
    name = strtok(NULL,"'");
  }
  return dir;
}
HINDEX head_find(char* path,U32 len,HINDEX* searchlist){
  if('\''==*path)
    return head_find_absolute(path,len);
  HINDEX dir;
  while(dir=*searchlist++){
    HINDEX ret;
    if(ret=head_locate(dir,path,len))
      return ret;
  }
  return 0;
}


void head_dump_one(HINDEX h){
  sHeader*p = &HEAD[h];
  // next dad child type table
  printf("%4d %4x %4x %4x type:%4x ->:%8x parm:%d [%s]\n",
         h,p->next,p->dad,p->child,p->type,p->pcode,p->parm,p->name);
}