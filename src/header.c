#include <string.h>

#include "global.h"
#include "header.h"
#include "data.h"
//=======================================================
// sHeader structure is private.  Headers can be reworked
// at a later time.
typedef struct sHeader {
        struct sHeader* next;               //header index
        struct sHeader* dad;
        struct sHeader* child;
        struct sHeader* type;               //points at type directory
        TOKEN* pcode;              //headers refer to actual code
        //
        PARM  parm;                //decompilation data
        U8 namelen;                //actual name part of string
        U16 unused_a;              //padding                   
        //
        // a name follows inline
        char name[];
        
} sHeader;
typedef sHeader* HINDEX;
/*
 * Head dictionary is a linear structure containing sHeaders...


*/

U8*            phead;           //U8* since sHeader is variable length...
HINDEX hroot ;                  //hroot is special...

void head_init(U8*start, U32 size){
        phead=start;
        hroot = (HINDEX)phead;
printf("head_init: root is %p\n",hroot);

}

HINDEX head_get_root(){
    return hroot;
}

/*
    Create a new header.  
*/
HINDEX head_new(char* src,U32 cnt, U8*pcode,HINDEX type,PARM parm,HINDEX dad)
{
    HINDEX head = (HINDEX)phead;
    
  head->dad = dad;
  head->child = 0;
  head->type = type;
  head->pcode = pcode;
  head->parm = parm;
  //copy name and comment inline, null-term...
  strncpy(head->name,src,cnt);
  head->name[cnt] = 0;
  phead = (U8*)(head->name+cnt+1);
  
  //calculate name size
  char*sep = strpbrk(head->name," \t\r\n");
  U32 namelen = sep?sep-head->name:cnt;
  if(namelen>255) {
      src_error("head_new: name too long %d [%s]\n",namelen,head->name);
      return 0;
  }
  head->namelen = namelen;
//printf("head_new: (%.*s)\n",head->namelen,head->name);
  
  //link in
    head->next = dad?dad->child:0;      //dad's first child is our sib
    if(dad)
        dad->child=head;                //we are dad's first child
  
  return head;
}

HINDEX head_locate(HINDEX dir,char* name,U32 len){   
//printf("head_locate: dir is %p[%s]\n",dir,dir->name);      
//printf("head_locate: child is %p\n",dir->child);      
    HINDEX h = dir->child;
    while(h){
        if(len == h->namelen)
            if(0==strncmp(name,h->name,len))
                return h;
if(h->next==h){
  printf("ERROR: %s's next is itself!\n",h->name);
  exit(0);
}
        h = h->next;
    
  }
  
  return h;
}
extern HINDEX H_PROC;
HINDEX head_find_or_create(char* path){
//printf("head_find_or_create [%s]\n",path);
  HINDEX dir = head_get_root();         //start at root
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
            case '\r':
            case '\n':
              return cnt;
            default:
              cnt++;
        }
    }
}

/* ============================================================================
*  Find an absolute path
*/
HINDEX head_find_absolute( char* ptr,U32 ulen){
//printf("head_find_absolute[%s] %d\n",ptr,ulen);
  HINDEX dir = head_get_root();         //start at root
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
*  Find an absolute path or die
*/
HINDEX head_find_abs_or_die( char* path){
    HINDEX ret = head_find_absolute(path,strlen(path));
    if(!ret){
        fprintf(stderr,"ERROR: head_find_abs_or_die cannot find [%s]\n",path);
        exit(1);
    }
}
/* ============================================================================
*  Find using a searchlist
*/
HINDEX head_find(char* ptr,U32 len,HINDEX* searchlist){
//printf("head_find[%s] %d\n",ptr,len);
    if('\''==*ptr)
    return head_find_absolute(ptr+1,len);
  HINDEX dir;
//printf("head_find dir is [%d] \n",*searchlist);
  while(dir=*searchlist++){
//printf("head_find[%s] trying directory %s \n",ptr,head_get_name(dir));
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
     //TODO:
printf("head_save_one NOT IMPLEMENTED\n");
exit(0);
/*//printf("\n%p :",ptr);
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
*/}

const char* head_get_name(HINDEX h){
    return h->name; 
}
U32 head_get_namelen(HINDEX h){
    return h->namelen;
}
HINDEX head_get_child(HINDEX h){
    return h->child;
}
HINDEX head_get_next(HINDEX h){
    return h->next;
}
HINDEX head_get_dad(HINDEX h){
    return h->dad;
}
TOKEN* head_get_code(HINDEX h){
    return h->pcode;
}
PARM head_get_parm(HINDEX h){
    return h->parm;
}

void    head_set_type(HINDEX h,HINDEX type){
    h->type = type;
};
void    head_set_parm(HINDEX h,PARM parm){
    h->parm = parm;
}
void    head_set_code(HINDEX h,TOKEN* code){
    h->pcode = code;
}


void head_dump_one(HINDEX h){
  sHeader*p = h;
  // next dad child type table
  printf("%p (next)%p (dad)%p (child)%p type:%p ->:%8x parm:%d [%s]\n",
         p,p->next,p->dad,p->child, p->type,(U32)p->pcode,p->parm,p->name);
}

/* ============================================================================
 * save
 * 
 * format:
 * -32-bit count
 * 
 * -sheader
 * -32-bit text length
 * -text
 * ...
 */

int head_save_one(FILE* f,HINDEX h){
    //TODO:
printf("head_save_one NOT IMPLEMENTED\n");
exit(0);
/*    U32 ret = 0;
    ret+=fwrite(&HEAD[h], sizeof(sHeader)-4, 1, f );
    U32 textlen = strlen(h->name)+1;
    ret+=fwrite(&textlen,4,1,f);
    ret+=fwrite(h->name,textlen,1,f);
    return (ret==3)?1:0;
*/ 
}


int head_load_one(FILE* f,HINDEX h){
    //TODO:
printf("head_load_one NOT IMPLEMENTED\n");
exit(0);
   
    
}
int head_save(FILE* f){
    //TODO:
printf("head_save NOT IMPLEMENTED\n");
exit(0);
/*    U32 cnt = hindex_last;
    if(1 != fwrite(&cnt,4,1,f)) return 0;
    int i;
    for(i=0;i<cnt;i++){
        if(1 != head_save_one(f,i)) return 0;
    }
    return 1;
*/    
}
