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
    PTOKEN* ptr = table_base(address);          // 0 is ok, just 
    U32 i;
    for(i=0;i<=255;i++){                        //for every possible token
        if(NULL==*ptr++) return ptr-1;              //if address 0, we are done
    }
    return 0;
}
/* ============================================================================
 * table_wipe
 * 
 * clear the table from table address on
 */
void table_wipe(PTOKEN* address){
    memset(address,0,((U32)var->table_top - (U32)address));
}

void table_dump(PTOKEN* p){
 printf("ok\n");
    int i; for(i=0;i<16;i++){
        printf("%2d %p: %p \n",i,p,*p);
        p++;
    }
}


