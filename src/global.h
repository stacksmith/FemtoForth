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
#include  <stdio.h>// printf
#include <stdlib.h>

#include <sys/mman.h>

typedef unsigned int U32;
typedef unsigned short U16;
typedef unsigned char U8;
typedef char S8;
struct sHeader;

typedef struct sHeader* HINDEX;        //header index
typedef U32 TINDEX;        //table index
typedef U8  PARM ;         //token stream parameter

typedef unsigned char TOKEN;
typedef TOKEN* PTOKEN;

#define T_PROC  0     
#define T_U8    1
#define T_U16   2
#define T_U32   3
#define T_OFF   4
#define T_STR8  5
#define T_REF   6
#define T_DIR   255
// must match kernel definitions

//=======================================================
//GLOBAL SETTINGS
//=======================================================
#define CODE_ADDRESS 0x04000000
#define CODE_SIZE    0x01000000
#define HEAD_SIZE    0x00100000
//bottom 512 bytes reserved for variables
#define HOST_RESERVED    128    // 32 host-based pointers (lay
#define SYS_RESERVED     128    // 32 system pointers
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

typedef struct sMemLayout {
/* 00  0 */ 
            U8* data_base;                
/* 04  4 */ 
            U8* data_top;
/* 08  8 */ 
            U8* table_base;
/* 0C 12 */ 
            U8* table_top;
/* 10 16 */ 
            U8* dsp_base;
/* 14 20 */ 
            U8* dsp_top;
/* 18 24 */  
            U8* rsp_base;
/* 1C 28 */ 
            U8* rsp_top; 
/* 20 32 */ 
            U8* head_base; 
/* 24 36 */           
            U8* head_top; 
/* 28 40 */           
            char* src_base; 
/* 2C 44 */           
            char* src_top;             
} sMemLayout;


struct sHeader;
//=======================================================
//System variable structure.  It is placed at the bottom
// of the data section, and used by both compiler and
// system
//=======================================================
// DANGER: update bindings.asm and kernel.asm if offset change!!!
// at DATA_BASE+HOST_RESERVED (currently 0x80
typedef struct sVar {
/* 00  0 */ 
            U8* sp_c;                   //c sp with context..
/* 04  4 */ 
            sRegsMM* sp_meow;           //our context
/* 08  8 */ 
            TOKEN* data_ptr;
/* 0C 12 */ 
            TOKEN* run_ptr;
/* 10 16 */ 
            PTOKEN* run_table;          //preserving table during run
/* 14 20 */ 
            char*  src_ptr;
/* 18 24 */ 
            U8* head_ptr; 
/* 1C 28 */ 
/* 4C 76 */
            struct sHeader* wd;

} sVar;

