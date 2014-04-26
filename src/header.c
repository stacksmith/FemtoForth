#include "global.h"
#include "header.h"
#include "data.h"
extern sHeader*       HEAD;
HINDEX hindex_last = 0;
//TODO: error-check allocation
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
  //name and comment
  header->pname = strndup(name,cnt);
  char*pcomment = strpbrk(name," \t\r\n");
  header->namelen = pcomment?pcomment-name:cnt;
printf("head_new: (%s) %d\n",name,header->namelen);
  
  //now actually count up the 
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
        if(len == HEAD[h].namelen)
            if(0==strncmp(name,HEAD[h].pname,len))
                return h;
if(HEAD[h].next==h){
  printf("ERROR: %s's next is itself!\n",HEAD[dir].pname);
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

U32 substr_cnt(char* str){
    U32 cnt = 0;
    while(1){
        switch(*str++){
            case 0: 
            case '\'':
            case ' ':
              return cnt;
            default:
              cnt++;
        }
    }
}

/* ============================================================================
*  Find an absolute path
*/
HINDEX head_find_absolute(char* ptr,U32 ulen){
//printf("head_find_absolute[%s] %d\n",ptr,ulen);
  HINDEX dir = 1;         //start at root
  U32 cnt=0;
  int len=ulen;
  while(len>0){
      cnt=substr_cnt(ptr);
      len=len-cnt-1;
//printf("head_find_absolute 1: [%s] %d\n",ptr,cnt);
    HINDEX found = head_locate(dir,ptr,cnt);
    if(!found) {
//printf("head_find_absolute: COULD NOT FIND [%s] %d\n",ptr,cnt);
    
      return 0;
    }
    dir = found;
    ptr=ptr+cnt+1;
  }
  return dir;
}
/* ============================================================================
*  Find using a searchlist
*/
HINDEX head_find(char* ptr,U32 len,HINDEX* searchlist){
//printf("head_find[%s] %d\n",ptr,len);
    if('\''==*ptr)
    return head_find_absolute(ptr+1,len);
  HINDEX dir;
  while(dir=*searchlist++){
    HINDEX ret;
    if(ret=head_locate(dir,ptr,len))
//printf("head_find found [%s] %d\n",ptr,len);
      return ret;
  }
  return 0;
}
/* ============================================================================
*  Resolve   given a pointer, find header
* 
*  We will find the header pointing at the pointer, or the nearest one below,
*  in case we are pointing inside a routine.  The highest one wins, to allow
*  for overloading.
* 
*/
HINDEX head_resolve(TOKEN* ptr,U32* poffset){
//printf("\n%p :",ptr);
//printf("head_resolve %p %p\n",ptr,poffset);
    if(!ptr){
        if(poffset) *poffset=0;
        return 0;
    }
    HINDEX h;
    HINDEX best_hindex=0;
    U32    best_offset = 0xFFFFFFFF;
    //search from the top for overdefining (for now)
    for(h = hindex_last;h >= 1; h--){
        TOKEN* p = HEAD[h].pcode;       //header points here.
        //since we are scanning down, it is always safe to
        //return the topmost matching one.
        if(!p) continue;
        if(p == ptr) {
            if(poffset) *poffset = 0;   //if offset requested, 0 it is.
            return h;
        }
        //if head points below the pointer, track offset (if it's better)
        if(p < ptr) {
            if((ptr-p)<best_offset){
                best_offset = (ptr-p);
                best_hindex=h;
            }
        }
    }
    //finally, return the best match
    if(poffset) *poffset = best_offset;
    return best_hindex;
}

const char* head_get_name(HINDEX h){ return HEAD[h].pname; }

void head_dump_one(HINDEX h){
  sHeader*p = &HEAD[h];
  // next dad child type table
  printf("%4d %4x %4x %4x type:%4x ->:%8x parm:%d [%s]\n",
         h,p->next,p->dad,p->child,p->type,p->pcode,p->parm,p->pname);
}