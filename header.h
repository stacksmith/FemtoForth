//=======================================================
#define NAMELEN 16
typedef struct sHeader {
        HINDEX next;               //header index
        HINDEX dad;
        HINDEX child;
        HINDEX type;               //points at type directory
        TINDEX index;              //table index, 0-based
        PARM  parm;            //decompilation data
        char name[NAMELEN];
} sHeader;

HINDEX  head_new(char* name, TINDEX index,HINDEX type,PARM parm, HINDEX dad);
void    head_build();
HINDEX head_locate(HINDEX dir,char* name,U32 len);
HINDEX head_find_or_create(char* path);
HINDEX head_find_absolute(char* path,U32 len);

void head_dump_one(HINDEX h);
