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
// Note: index 0 is not used (so not found = 0).  Index 1 is root
#include "global.h"
#include "header.h"
extern sVar* var;
extern sMemLayout* lay;
/* ============================================================================
 * table_base
 * TODO:***
 * Determine the accessible base for a pointer
 */

TOKEN** table_base(TOKEN* p){
    TOKEN**tbase = (TOKEN**)(((U32)(p+1) >>2) & 0xFFFFFFFC);
    return tbase;
}

TOKEN* table_base_inverted(TOKEN** ptable){
    TOKEN* ret = (TOKEN*)((((U32)ptable)<<2)-1);
    //note: may be 1 less then bottom!
    return ret;
}

extern HINDEX H_PROC;           //in interpreter

/* ============================================================================
 * table_find_or_create
 * 
 * given a compile address, find a token representing an address.
 * If not found, create one.
 * Return token, or 0 if cannot find or create!
 */
TOKEN table_find_or_create(TOKEN* address,TOKEN* target){
    TOKEN**base = table_base(address);
    TOKEN tok;
    for(tok=1;tok<=255;tok++){ //for every possible token value
        if(target==base[tok]) { //does the table already have a reachable?
//printf("table_find_or_create: found an entry, token %02x at %08x\n",tok,(U32)address);
           return tok;
        } else {
            if(NULL == base[tok]) { //empty slot?
                base[tok] = target;    //create a slot entry
//printf("table_find_or_create: created an entry, token %02x at %08x\n",tok,(U32)address);
                return tok;
            }   
        }
    }
    return 0;    
}

/* ============================================================================
 * table_end
 * 
 * given a compile address, find the top 
 */
PTOKEN* table_end(PTOKEN address){
    PTOKEN* ptr = table_base(address)+255;          
    U32 i;
    if(*ptr) {//TODO: handle this
printf("table_end ERROR: no room intable!\n");
    }
    for(i=255;i>=1;i--,ptr--){  
        if(NULL!=*ptr) return ptr+1;              //if address 0, we are done
    }
    return ptr+1;
}


/* ============================================================================
 * table_cleanse
 * 
 * When a block of code is deleted, we can't just wipe the table (for a number
 * of reasons - entries may be used by previous code, etc). 
 * 
 * The solution is to scan the reachable range for a code range erased, and
 * eliminate unused entries...
 
void table_wipe(TOKEN* start,TOKEN* end){
    //find lowest and highest reachable table slots.
    PTOKEN* lowest = table_base(start)+1;       //0 is not a usable token
    PTOKEN* highest = table_base(end)+255;
    memset(address,0,((U32)var->table_top - (U32)address));
}

*/
void table_wipe(TOKEN* address){
   memset(address,0,((U32)lay->table_top - (U32)address));
}

int table_ptr_verify(PTOKEN*p){
    if(p < (PTOKEN*)lay->table_bottom) return 0;
    if(p >= (PTOKEN*)lay->table_top) return 0;
    return 1;
}

PTOKEN* table_dump(PTOKEN* p){
    int i; for(i=0;i<16;i++){
        printf("\33[0;32m");
        printf("%2d %p: ",i,p);
        if(!p) printf("0");
       
        if(table_ptr_verify(p)){
            // output hindex,address, target and name
            U32 offset=0;
            HINDEX h = head_resolve(*p,&offset);
            if(*p){
                printf("%.08X %.0d \33[0;33m%.*s",(U32)*p,offset,head_get_namelen(h),head_get_name(h));
            }
        }
        printf("\n");
        printf("\33[0;37m");
        p++;
    }
    return p;
}
U32 table_count_used(PTOKEN*ptab){
    //0 is not used, so start at 1...
    int i;
    U32 count = 0;
    for(i=1;i<256;i++){
        if(ptab[i]) count++;
    }
    return count;
}



/* ============================================================================
//  * table_cleanse
 * 
 * When a block of code is deleted, we can't just wipe the table (for a number
 * of reasons - entries may be used by previous code, etc). 
 * 
 * A crude but workable solution is to preserve the top of the working table
 * and clear up from it after a run.  This works for a narrow range of situations
 * and requires no holes in table, bottom-up filling etc.
 * 
 */


/* ============================================================================
 * codestream_seq

 Step through every header, from the bottom up.
 For every header, step through every valid bytecode.
 For every bytecode, call proc
 
 -Skip blobs and non-bytecode storage in data...
 -Skip 0 tokens indicating implied return;
 -Skip all in-line parameters of bytecodes
 */
// called for every head in sequence
typedef struct sCodeStreamParams{
    codestream_proc proc;
    void* theParams;
} sCodeStreamParams;

//A dump of the map after the count of used table entries...
void tbl_cln_report(U8* map,U32 cnt){
    U32 index;
    U8** table = (U8**)lay->table_bottom;
    for(index=0;index<cnt;index++){
        U8* ptr = table[index];
        HINDEX h = head_owner(ptr);
        printf("%d %p %p \t%.*s\t%d\n",index,
               table+index,table[index],
               
               head_get_namelen(h),head_get_name(h),
               map[index]);
    }
}

int codestream_head_proc(HINDEX h,void*p){
    TOKEN* ptok = head_get_code(h);
    TOKEN* end  = ptok + head_get_datasize(h);
    if(!ptok) return 0;   //dirs have 0 code pointers
    if(head_get_blob(h)) return 0; //blobs not interesting
    while(ptok < end){
        TOKEN** base = table_base(ptok);
        TOKEN tok = *ptok++;
        if(tok) { //don't count implicit returns
//  printf("A %p %02X  \n",ptok,tok);
     
            // now figure out how to skip...
            U8* target =base[tok];
            HINDEX owner = head_owner(target);
if(!owner){
    printf("CANNOT FIND owner for token %d at %p\n",
        tok,ptok-1);
    return 1;
}            
            HINDEX type = head_get_type(owner);
            //invoke the procedure on this token
            {
  //printf("B %p %p  \n",p1->proc);
               sCodeStreamParams*p1 =(sCodeStreamParams*)p;
               int ret = p1->proc(ptok-1,target,owner,type,p1->theParams);
               if(ret) return ret; //inner process terminated...
 // printf("c %p %02X  \n",ptok,tok);
            }
            switch(head_get_ptype(owner)){
                case PAYLOAD_NONE: break;
                case PAYLOAD_ONE:  ptok++; break;
                case PAYLOAD_TWO:  ptok+=2; break;
                case PAYLOAD_FOUR: ptok+=4; break;
                case PAYLOAD_OFF8: ptok+=1; break;
                case PAYLOAD_REF:  ptok++;  break;
                case PAYLOAD_STR8: 
                    { U32 len = *ptok++;
                        ptok += len;
                        break;
                    }
                default: 
                    printf("codestream_head_proc: invalid payload type %d\n",
                            head_get_ptype(type));
                    return 1;
            } 
        }//else tok=0, as in ;
//printf("B\n");
    }
    return 0;
}
void codestream_seq(codestream_proc func,void* params){
    sCodeStreamParams innerParams = {func,params};
    head_seq(codestream_head_proc,&innerParams);
    
}

int test_proc(TOKEN* ip,TOKEN* target,HINDEX owner,HINDEX type,void* params){
 //printf("%p %02X  \n",ip,tok);
 return 0;   
}
//-----------------------------------------------------------------------------
// increment the count for our token in the bytemap
void tbl_cln_add(U8* map,TOKEN** base,TOKEN tok){
    U32 index = ((U32)base - (U32)(lay->table_bottom)) /4 + tok;
//printf("adding index %d\n",index);
    if(map[index] <255)
        map[index]++;
}
//-----------------------------------------------------------------------------
// procedure to tabulate table usage...
int table_clean_proc(TOKEN*ip, TOKEN*target,HINDEX owner,HINDEX type,void* map){
    TOKEN** base = table_base(ip);
    TOKEN tok = *ip;
    tbl_cln_add((U8*)map,base,tok);
    //check for ref type and handle its parameter as well..
    if(PAYLOAD_REF == head_get_ptype(owner)){
        ip++;
        TOKEN** base = table_base(ip);
        TOKEN tok = *ip;
        tbl_cln_add((U8*)map,base,tok);
    }
    return 1;
}
//-----------------------------------------------------------------------------
// 
#include <sys/time.h>
int table_clean(PTOKEN*p ){
    // prepare a map with a byte count for every table entry
    U32 mapsize = (((U32)(table_base(var->data_ptr)+256)) - (U32)(lay->table_bottom))/4;
    U8* map = (U8*)malloc(mapsize);
    memset(map,0,mapsize);
    // and process each header...
    struct timeval time_in,time_out;
    gettimeofday(&time_in,0);
    codestream_seq(table_clean_proc,map);
    gettimeofday(&time_out,0);
  
    tbl_cln_report(map,mapsize);
//printf("table_clean: table %x\n",mapsize);
    free(map);
    
    if(time_out.tv_usec < time_in.tv_usec)
        time_out.tv_sec+=1;
    time_out.tv_usec -= time_in.tv_usec;
    time_out.tv_sec  -= time_in.tv_sec;
printf("ELAPSED: %u.%06d\n",(U32)time_out.tv_sec,(U32)time_out.tv_usec);
    return 1;
}

/* ============================================================================
 * table_seq

 Step through every table element, invoking proc
 */
PTOKEN* table_seq(table_proc func,void* params){
    //start at table bottom,TODO: skip unused pointer area indata
    PTOKEN* p = (PTOKEN*)lay->table_bottom;
    PTOKEN* ptop = table_base(var->data_ptr)+256;     
    while(p < ptop){
        int ret = func(p,params);
        if(ret) return p;
        p++;
    }
    return 0;
}

/* ============================================================================
 * table_cnt_refs(PTOKEN*p)
 * Walk the tabe and find all references to our target.
 * */

//-----------------------------------------------------------------------------
typedef struct sTableCntRefs {
    TOKEN* target;
    U32 cnt;
} sTableCntRefs;

int table_cnt_refs_p(TOKEN*ip, TOKEN* target,HINDEX owner,HINDEX type,void* data){
    sTableCntRefs* s = (sTableCntRefs*)data;
    
//printf("%p %p %p \n",ip,target, s->target);
    if(target == s->target){
        s->cnt++;                //tabulate references...
        // a reference is found... 
        U32 off;
        HINDEX container = head_resolve(ip,&off);
        printf("%04d + ",off);
        head_ls(container);
    }
    return 0; //continue.
}
int table_cnt_refs(){
    //step through the table...
    sTableCntRefs s = {(TOKEN*)dstack_pop(),0}; //target on datastack.
    codestream_seq(&table_cnt_refs_p,&s);
printf("%d reference(s) found\n",s.cnt);
    return 1;
}

//-----------------------------------------------------------------------------
//show all table entries...
int table_xxx_p(PTOKEN* tabptr,void*params){
    printf("%p, %p\n",tabptr,*tabptr);
    return 0;
}
int table_xxx(){
    table_seq(&table_xxx_p,0);
    return 1;
}
