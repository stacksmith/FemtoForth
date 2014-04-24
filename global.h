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
typedef struct sRegsC {
    U32 r4;
    U32 r5;
    U32 r6;
    U32 r7;
    U32 r8;
    U32 r9;
    U32 r10;
    U32 r11;
    U32 lr;
}sRegsC;

typedef struct sRegsMM {
    U32 r0;     //TOS
    U32 r6;     //IP
    U32 r7;     //DSP
    U32 r9;     //ER
    U32 r11;    //DAT
    U32 lr;      //interpreter pointer
} sRegsMM;
//=======================================================
//System variable structure.  It is placed at the bottom
// of the data section, and used by both compiler and
// system
//=======================================================
typedef struct sVar {
/* 0  */  U8* data_base;                /* 0 */
/* 4  */  U8* data_top;
/* 8  */  U8** table_base;
/* 12 */  U8* table_top;
/* 16 */  U8* dsp_base;
/* 20 */  U8* dsp_top;
/* 24 */  U8* rsp_base;
/* 28 */  U8* rsp_top; 
/* 32 */  U8* htable_base;
/* 36 */  U8* htable_top;

  
/* 40 */  U8* data_ptr;
/* 44 */  U8** table_ptr;
/* 48 */  U8* run_ptr;
/* 52 */  U8* sp_c;                   //c sp with context..
/* 56 */  sRegsMM* sp_meow;
/* 60 */  U8* unused2;
/* 64 */  U8* unused3;
/* 68 */  U8* unused4;
} sVar;

