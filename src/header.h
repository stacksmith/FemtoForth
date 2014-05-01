void head_init(U8*start, U32 size);

HINDEX head_get_root();
U32     head_get_namelen(HINDEX h);
HINDEX  head_get_child(HINDEX h);
HINDEX  head_get_next(HINDEX h);
HINDEX  head_get_dad(HINDEX h);
TOKEN*          head_get_code(HINDEX h);
PARM            head_get_parm(HINDEX h);
const char*     head_get_name(HINDEX h);

void    head_set_type(HINDEX h,HINDEX type);
void    head_set_parm(HINDEX h,PARM parm);
void    head_set_code(HINDEX h,TOKEN* code);

HINDEX  head_new(char* name,U32 cnt, U8* pcode,HINDEX type,PARM parm, HINDEX dad);

void    head_build();
HINDEX head_find(char* ptr,U32 len,HINDEX* searchlist);

HINDEX head_locate(HINDEX dir,char* name,U32 len);
HINDEX head_find_or_create(char* path);
HINDEX head_find_absolute( char* path,U32 len);
HINDEX head_find_abs_or_die( char* path);
HINDEX head_resolve(TOKEN* ptr,U32* poffset);

void head_dump_one(HINDEX h);
int head_save(FILE* f);