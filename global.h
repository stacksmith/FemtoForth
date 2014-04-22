#include  <stdio.h>// printf
#include <stdlib.h>

#include <sys/mman.h>

typedef unsigned int U32;
typedef unsigned short U16;
typedef unsigned char U8;

typedef U16 HINDEX;        //header index
typedef U32 TINDEX;        //table index
typedef U8  PARM ;         //token stream parameter

#define T_NA    0     
#define T_U8    1
#define T_U16   2
#define T_U32   3
#define T_OFF   4
#define T_STR   5



//=======================================================
//GLOBAL SETTINGS
//=======================================================
#define CODE_ADDRESS 0x04000000
#define CODE_SIZE    0x01000000
#define HEAD_MAX  10000



