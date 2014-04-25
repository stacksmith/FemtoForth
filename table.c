// Note: index 0 is not used (so not found = 0).  Index 1 is root
#include "global.h"
extern sVar* var;
/* ============================================================================
 * table_base
 * 
 * Determine the accessible base for a pointer
 */

TOKEN** table_base(TOKEN* p){
    TOKEN**tbase = (TOKEN**)(((U32)(p+1) >>2) & 0xFFFFFFFC);
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
printf("table_find_or_create: found an entry, token %02x at %08x\n",tok,address);
           return tok;
        } else {
            if(NULL == base[tok]) { //empty slot?
                base[tok] = target;    //create a slot entry
printf("table_find_or_create: created an entry, token %02x at %08x\n",tok,address);
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
 * table_wipe
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
   memset(address,0,((U32)var->table_top - (U32)address));
}


void table_dump(PTOKEN* p){
 printf("ok\n");
    int i; for(i=0;i<16;i++){
        printf("%2d %p: %p \n",i,p,*p);
        p++;
    }
}


