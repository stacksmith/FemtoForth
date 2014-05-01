#include  <stdio.h>// printf
#include <stdlib.h>

#include <sys/mman.h>

typedef unsigned int U32;
typedef unsigned short U16;
typedef unsigned char U8;
struct sHeader;

typedef struct sHeader* HINDEX;        //header index
typedef U32 TINDEX;        //table index
typedef U8  PARM ;         //token stream parameter

typedef unsigned char TOKEN;
typedef TOKEN* PTOKEN;

#define T_NA    0     
#define T_U8    1
#define T_U16   2
#define T_U32   3
#define T_OFF   4
#define T_STR   5
#define T_REF   6


//=======================================================
//GLOBAL SETTINGS
//=======================================================
#define CODE_ADDRESS 0x04000000
#define CODE_SIZE    0x01000000
#define HEAD_SIZE    0x00100000
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
    U32 TOS; //r0;     //TOS
    U32 IP;  //r6;     //IP
    U32* DSP; //r7;     //DSP
    U32 ER;  //r9;     //ER
    U32 DAT; //ar11;    //DAT
    U32 lr;      //interpreter pointer
} sRegsMM;


//=======================================================
//System variable structure.  It is placed at the bottom
// of the data section, and used by both compiler and
// system
//=======================================================
// DANGER: update bindings.asm and kernel.asm if offset change!!!
typedef struct sVar {
/* 0 */  U8* sp_c;                   //c sp with context..
/* 4 */  sRegsMM* sp_meow;
/* 8  */  U8* data_base;                /* 0 */
/* 16  */  U8* data_top;
/* 20 */  U8* table_base;
/* 24 */  U8* table_top;
/* 28 */  U8* dsp_base;
/* 32 */  U8* dsp_top;
/* 36 */  U8* rsp_base;
/* 40 */  U8* rsp_top; 
/* 44 */  U8* head_base; 
/* 48 */  U8* head_top; 
/* 52 */  U8* head_ptr; 

//U8* shit;  
/* 56 */  TOKEN* data_ptr;
/* 60 */  TOKEN* run_ptr;
/* 64 */  PTOKEN* run_table;          //preserving table during run
          U32 terminator;             //set to non-0 as end of table
} sVar;

