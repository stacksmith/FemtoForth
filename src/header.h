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
void head_init(U8*start, U32 size);

HINDEX head_get_root();
U32     head_get_namelen(HINDEX h);
HINDEX  head_get_child(HINDEX h);
HINDEX  head_get_next(HINDEX h);
HINDEX  head_get_dad(HINDEX h);
HINDEX  head_get_type(HINDEX h);
U32     head_get_datasize(HINDEX h);


char* head_get_source(HINDEX h);
char* head_get_comments(HINDEX h,U32* len);


TOKEN*          head_get_code(HINDEX h);
char*     head_get_name(HINDEX h);
char* head_name(HINDEX h,U32*size);

void    head_set_type(HINDEX h,HINDEX type);
void    head_set_code(HINDEX h,TOKEN* code);
void    head_set_datasize(HINDEX h,U32 datasize);

HINDEX  head_new(U8* pcode,HINDEX type, HINDEX dad);
HINDEX head_append_source(HINDEX h,char* buf,U32 cnt);
HINDEX head_commit(HINDEX h);

void    head_build();
HINDEX head_find(char* ptr,U32 len,HINDEX* searchlist);

HINDEX head_locate(HINDEX dir,char* name,U32 len);
HINDEX head_find_or_create(char* path);
HINDEX head_find_absolute( char* path,U32 len);
HINDEX head_find_abs_or_die( char* path);

HINDEX head_resolve(TOKEN* ptr,U32* poffset);
HINDEX head_owner(TOKEN* ptr);

void head_dump_one(HINDEX h);

int head_save(FILE* f);
HINDEX head_nextup(HINDEX h);