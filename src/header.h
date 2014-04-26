//=======================================================
#define NAMELEN 16
typedef struct sHeader {
        HINDEX next;               //header index
        HINDEX dad;
        HINDEX child;
        HINDEX type;               //points at type directory
        TOKEN* pcode;              //headers refer to actual code
        //
        PARM  parm;                //decompilation data
        U8 namelen;                //actual name part of string
        U8 unused_a;
        U8 unused_c;
        //
        const char *pname;               //malloc'd name
        
} sHeader;

void    head_set_segment(void* ptr);

U32     head_get_namelen(HINDEX h);
HINDEX  head_get_child(HINDEX h);
HINDEX  head_get_next(HINDEX h);
HINDEX  head_get_dad(HINDEX h);
TOKEN* head_get_code(HINDEX h);

void    head_set_type(HINDEX h,HINDEX type);
void    head_set_parm(HINDEX h,PARM parm);
void    head_set_code(HINDEX h,TOKEN* code);

HINDEX  head_new(char* name,U32 cnt, U8* pcode,HINDEX type,PARM parm, HINDEX dad);
void    head_build();
HINDEX head_locate(HINDEX dir,char* name,U32 len);
HINDEX head_find_or_create(char* path);
HINDEX head_find_absolute(char* path,U32 len);
HINDEX head_resolve(TOKEN* ptr,U32* poffset);
const char*  head_get_name(HINDEX h);

void head_dump_one(HINDEX h);
int head_save(FILE* f);