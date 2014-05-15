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
#include <string.h>

#include "global.h"
#include "header.h"
#include "data.h"

extern sVar*            var;
extern sMemLayout*       lay;
//=======================================================
// sHeader structure is private.  Headers can be reworked
// at a later time.
typedef struct sHeader {
        struct sHeader* next;      // 0         //header index
        struct sHeader* dad;       // 4
        struct sHeader* child;     // 8
        struct sHeader* type;      // 12         //points at type directory
        TOKEN* pcode;              // 16  headers refer to actual code
        U32    timestamp;          // 20
        U16    datasize;           // 24
        U16 srclen;                // 26  
        
        U8 pad1;                   // 28
        U8 entry_flag;              // 29
        U8 payload_flag;                  // 30
        U8 namelen;                // 31 actual name part of string
        //                         // 
        // a name follows inline
        char src[];
        
} sHeader;
typedef sHeader* HINDEX;
//
// Flag contains payload type:
/*
 * Head dictionary is a linear structure containing sHeaders...


*/



HINDEX head_get_root(){
    return (HINDEX)lay->head_bottom;
}
U32 head_size(HINDEX h){
    return sizeof(sHeader) + h->srclen;
}
/*
    Create a new header.  Sequence:
    -head_new... to create header
    -head_append_source to add source
    -head_commit to finish
*/
HINDEX head_new(U8*pcode,HINDEX type,HINDEX dad)
{
//printf("sizeof(sHeader) is %d\n",sizeof(sHeader));        
  HINDEX head = (HINDEX)var->head_ptr;
  head->dad = dad;
  head->child = 0;
  head->type = type;
  head->pcode = pcode;
  head->srclen = 0;
  head->namelen = 0;
  head->payload_flag = 0;
  head->entry_flag = 0;
  //link in
  head->next = dad?dad->child:0;      //dad's first child is our sib
  if(dad)
    dad->child=head;                //we are dad's first child
//printf("head_new: dad is %p\n",dad);
   // printf("offset of srccnt is %d\n",(U32)(&head->namelen)-(U32)head);
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
extern HINDEX H_PROC,H_DIR;
HINDEX head_find_or_create(char* path){
//printf("head_find_or_create [%s]\n",path);
  HINDEX dir = head_get_root();         //start at root
  char* name = strtok(path,"'");
  while(name){
    HINDEX found = head_locate(dir,name,strlen(name));
    if(!found) {
//printf("head_find_or_create  CREATING [%s] in dir %p\n",name,dir);
      dir = head_new(0,H_DIR,dir);
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
    HINDEX ret;
    if(ret=head_locate(var->wd,ptr,len))
        return ret;
    HINDEX dir;
    while(dir=*searchlist++){
    //printf("head_find[%s] trying directory %s \n",ptr,head_get_name(dir));
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
*  Resolve   given a pointer, find header.  Pointer may point inside data.
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
    HINDEX h=(HINDEX)lay->head_bottom;
    HINDEX best_hindex=h;
    U32    best_offset = 0xFFFFFFFF;
    //traverse the dictionary brute force style, disregarding hierarchy.
    while(h < (HINDEX)var->head_ptr){
//printf("head_resolve1 %p \n",h);
        TOKEN* target = h->pcode;       //header points here.
//printf("head_resolve2 %p \n",target);
        //if target is below the pointer, track offset (if it's better)
        if((target > lay->data_bottom) && (target <= ptr)) {
            if((ptr-target)<best_offset){
                best_offset = (ptr-target);
                best_hindex=h;
            }
        }
        //step to next head
        h = (HINDEX)(head_size(h) + (U32)h);
    }
    //finally, return the best match
    if(poffset) *poffset = best_offset;
    return best_hindex;
}
/* ============================================================================
*  Owner   given a pointer, find header that points directly at it...
* 
*/
int head_owner_proc(HINDEX h,void*p){
    if(p == head_get_code(h))
        return 1;
    return 0;
}
HINDEX head_owner(TOKEN* ptr){
    return head_seq(head_owner_proc,ptr);
/*    if(!ptr) return 0;
    HINDEX h=(HINDEX)lay->head_bottom;
    while((U8*)h < var->head_ptr){
        if(ptr == head_get_code(h))
            return h;
        h = (HINDEX)(head_size(h) + (U32)h);
    }
    return 0;
*/
}
/* ============================================================================
*  is_in
* 
*/
typedef struct sHeadIsIn{
    HINDEX best_h;
    TOKEN* ref;
    U32 best_dif;
} sHeadIsIn;

//----------------------------------------------------------
int head_is_in_proc(HINDEX h,void* _parm){
    sHeadIsIn* parm = (sHeadIsIn*)_parm;
   TOKEN* p = head_get_code(h);         
   if(p < parm->ref){                   //is head's pointer below reference?
       U32 dif = parm->ref - p;         //yup, here is the difference
       if(dif<= (parm->best_dif)) {     //is it better then our best difference?
           parm->best_dif = dif;        //yup, this is the one for now.
           parm->best_h = h;
       }
   }
   return 0;    //continue processing
}

HINDEX head_is_in(TOKEN* ref){
    sHeadIsIn data = {0,ref,0xFFFFFFFF};
    HINDEX ret = head_seq(&head_is_in_proc,&data);
    return data.best_h;
}

char* head_get_name(HINDEX h){
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
HINDEX  head_get_type(HINDEX h){
    return h->type;
}
U32 head_get_datasize(HINDEX h){
    return h->datasize;
}
char* head_get_source(HINDEX h){
    return h->src + h->namelen;
}

int head_get_ptype(HINDEX h){
    return h->payload_flag & 0xF;
}

void head_set_ptype(HINDEX h,U32 ptype){
    h->payload_flag = (h->payload_flag & 0xF0) | ptype;
}

int head_get_blob(HINDEX h){
    return h->entry_flag & 1;
}
void head_set_blob(HINDEX h,U32 ptype){
    h->entry_flag = (h->entry_flag & 0xFE) | (ptype & 1 ); 
}
char* head_name(HINDEX h,U32*size){
    char* ret = h->src;
    if(size) *size = h->namelen;
    return ret;
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
//printf("head_get_comments: length is %d [%.*s]\n",head_linelen(ret),head_linelen(ret),ret);
    return ret;
    
}

void    head_set_type(HINDEX h,HINDEX type){
    h->type = type;
};
void    head_set_code(HINDEX h,TOKEN* code){
    h->pcode = code;
}

void    head_set_datasize(HINDEX h,U32 datasize){
    h->datasize = datasize;
}

void head_dump_one(HINDEX h){
    sHeader*p = h;
//printf("---%d [%s]\n",head_get_namelen(p),head_get_name(p));
  // next dad child type table
  printf("\33[1;32m     PTR      NAME     NEXT      DAD    CHILD     TYPE      CODE    BASE  ENTRIES\33[0;32m\n");
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

HINDEX head_nextup(HINDEX h){
    return (HINDEX)( ((U32)h) + head_size(h) );
}

//typedef int (*head_proc)(HINDEX h, void* params);
// a process function receives header pointers and returns a bool.

HINDEX head_seq(head_proc func,void* params){
    HINDEX h=(HINDEX)lay->head_bottom;
    while(h < (HINDEX)var->head_ptr){
        int ret = func(h,params);
        if(ret) return h;
        h = (HINDEX)(head_size(h) + (U32)h);
    }
    return 0;
}
