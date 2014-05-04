#include <string.h>

#include "global.h"
#include "header.h"
#include "data.h"

extern sVar*   var;

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
        U16 srclen;               //padding                   
        //
        // a name follows inline
        char src[];
        
} sHeader;
typedef sHeader* HINDEX;

/*
 * Head dictionary is a linear structure containing sHeaders...


*/



HINDEX head_get_root(){
    return (HINDEX)var->head_base;
}
U32 head_size(HINDEX h){
    return sizeof(sHeader) + h->srclen+1;
}
/*
    Create a new header.  Sequence:
    -head_new... to create header
    -head_append_source to add source
    -head_commit to finish
*/
HINDEX head_new(U8*pcode,HINDEX type,PARM parm,HINDEX dad)
{
  HINDEX head = (HINDEX)var->head_ptr;
  head->dad = dad;
  head->child = 0;
  head->type = type;
  head->pcode = pcode;
  head->parm = parm;
  head->srclen = 0;
  head->namelen = 0;
  //link in
  head->next = dad?dad->child:0;      //dad's first child is our sib
  if(dad)
    dad->child=head;                //we are dad's first child
//printf("head_new: dad is %p\n",dad);
  return head;
}
/*
 * append source... if cnt=0, it is calculated.
*/
HINDEX head_append_source(HINDEX h,char* buf,U32 cnt){
    if(!cnt) cnt = strlen(buf);
    //append string to the end
    char* dest = ((char*)(h+1)) + h->srclen;
    memcpy(dest,buf,cnt);
    h->srclen += cnt;
    return h;
}
HINDEX head_commit(HINDEX h){
    //null-terminate source
    char* dest = ((char*)(h+1)) + h->srclen;
    *dest++=0;
    h->srclen++;
    //count the name size
    char*sep = strpbrk(h->src," \t\r\n");
    h->namelen = sep?sep - h->src: strlen(h->src);
    //and confirm
    var->head_ptr = (U8*)dest;
}


 /*(  //calculate name size
  char*sep = strpbrk(head->src," \t\r\n");
  U32 namelen = sep?sep-head->src:cnt;
  if(namelen>255) {
      src_error("head_new: name too long %d [%s]\n",namelen,head->src);
      return 0;
  }
  head->namelen = namelen;
//printf("head_new: (%.*s)\n",head->namelen,head->name);
*/
HINDEX head_locate(HINDEX dir,char* name,U32 len){  
//printf("head_locate %.*s: in dir  %p[%s]\n",len,name,dir,dir->name);      
    HINDEX h = dir->child;
    while(h){
        if(len == h->namelen)
            if(0==strncmp(name,h->src,len))
                return h;
if(h->next==h){
  printf("ERROR: %s's next is itself!\n",h->src);
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
//printf("head_find_or_create  CREATING [%s] in dir %p\n",name,dir);
      dir = head_new(0,H_PROC,T_NA,dir);
      head_append_source(dir,name,0);
      head_commit(dir);
//head_dump_one(dir);
/*U8* q = lang_ql((U8*)dir);
q=lang_ql(q);
q=lang_ql(q);
*/
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
*  search down from the directory specified
*/
HINDEX head_find_down(HINDEX dir, char* ptr,U32 ulen){
//printf("head_find_down[%s] %d\n",ptr,ulen);
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
*  Find initial entry.
* 
* It must be visible.
*/
HINDEX head_find_initial(char* ptr,U32 len,HINDEX* searchlist){
//printf("head_find_initial [%s] %d\n",ptr,len);
  HINDEX dir;
  while(dir=*searchlist++){
//printf("head_find[%s] trying directory %s \n",ptr,head_get_name(dir));
    HINDEX ret;
    if(ret=head_locate(dir,ptr,len))
      return ret;
  }
  return 0;
}
/* ============================================================================
*  Find using a searchlist
*/
HINDEX head_find(char* ptr,U32 len,HINDEX* searchlist){
//printf("head_find[%s] %d\n",ptr,len);
    if('\''==*ptr)
    return head_find_absolute(ptr+1,len);
//printf("head_find dir is [%d] \n",*searchlist);
  // find the initial term
  U32 cnt = substr_cnt(ptr);            //count the initial word
  HINDEX dir = head_find_initial(ptr,cnt,searchlist);
  if(!dir)      return 0;
  ptr+=cnt; len-=cnt;
  if(len)
  // now drill down
    dir = head_find_down(dir,ptr+1,len-1);      //account for the '
  return dir;
}
/*  while(dir=*searchlist++){
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
*  in case we are pointing inside a routine.  'The Price Is Right' rules apply.
* 
*/
HINDEX head_resolve(TOKEN* ptr,U32* poffset){
//sprintf("head_resolve %p %p\n",ptr,poffset);
    if(!ptr){ //TODO: can this happen?
        if(poffset) *poffset=0;
        return 0;
    }
    HINDEX h=(HINDEX)var->head_base;
    HINDEX best_hindex=h;
    U32    best_offset = 0xFFFFFFFF;
    //traverse the dictionary brute force style, disregarding hierarchy.
    while(h < (HINDEX)var->head_ptr){
//printf("head_resolve1 %p \n",h);
        TOKEN* target = h->pcode;       //header points here.
//printf("head_resolve2 %p \n",target);
        //if target is below the pointer, track offset (if it's better)
        if((target > var->data_base) && (target <= ptr)) {
            if((ptr-target)<best_offset){
                best_offset = (ptr-target);
                best_hindex=h;
            }
        }
        h = (HINDEX)(head_size(h) + (U32)h);
    }
    //finally, return the best match
    if(poffset) *poffset = best_offset;
    return best_hindex;
}

const char* head_get_name(HINDEX h){
    return h->src; 
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
HINDEX  head_get_type(HINDEX h){
    return h->type;
}

char* head_get_source(HINDEX h){
    return h->src + h->namelen;
}
//by convention, first line contains comments...
U32 head_linelen(char*p){
    char* pend = strpbrk(p,"\n\r\0");
    if(!pend) pend=p+strlen(p);
    return (pend-p);
    
}
char* head_get_comments(HINDEX h,U32* len){
    char* ret = head_get_source(h);
    if(len) 
        *len = head_linelen(ret);
    return ret;
    
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
//printf("---%d [%s]\n",head_get_namelen(p),head_get_name(p));
  // next dad child type table
  printf("\33[1;32m     PTR      NAME     NEXT      DAD    CHILD     TYPE      CODE    PARM     BASE  ENTRIES\33[0;32m\n");
  printf("%X ",(U32)p);
  printf("%9.*s",head_get_namelen(p),head_get_name(p));
  //         p,p->next,p->dad,p->child, p->type,(U32)p->pcode,p->parm,p->name);
  if(p->next)
    printf("%9.*s",head_get_namelen(p->next),head_get_name(p->next));
  else
    printf("     NONE");
  if(p->dad)
    printf("%9.*s",head_get_namelen(p->dad),head_get_name(p->dad));
  else
    printf("     ----");
  // child
  if(p->child)
    printf("%9.*s",head_get_namelen(p->child),head_get_name(p->child));
  else
    printf("     NONE");
  //type
  if(p->type)
    printf("%9.*s",head_get_namelen(p->type),head_get_name(p->type));
  else
    printf("     ----");
  printf("\33[0m\n");
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

