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
 * table_cleanse
 * 
 * When a block of code is deleted, we can't just wipe the table (for a number
 * of reasons - entries may be used by previous code, etc). 
 * 
 * A crude but workable solution is to preserve the top of the working table
 * and clear up from it after a run.  This works for a narrow range of situations
 * and requires no holes in table, bottom-up filling etc.
 * 
 */
int table_clean(PTOKEN*p ){
    //figure out the top of the usable table...
    U32 mapsize = (((U32)(table_base(var->data_ptr)+256)) - (U32)(lay->table_bottom))/4;
    U8* map = (U8*)malloc(mapsize);
    // walk the heads sequentially
    HINDEX h = (HINDEX)lay->head_bottom;
    while(h < (HINDEX)var->head_ptr){
        //head_dump_one(h);
        TOKEN* ptok = head_get_code(h);
        TOKEN* end = ptok + head_get_datasize(h);
printf("table_clean: %p %p\n",ptok,end);
        //Now, walk the bytecodes...
        TOKEN** base = table_base(ptok);
        TOKEN tok = *ptok++;
    
        
        h = head_nextup(h);

    }
//printf("table_clean: table %x\n",mapsize);
    free(map);
    return 1;
}
