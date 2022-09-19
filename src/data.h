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
void data_align4(void);
void data_align4_minus_1(void);
void data_align16_minus_1(void);
U8* data_compile_U8(U8 val);
U8* data_compile_U16(U16 val);
U8* data_compile_U32(U32 val);
U8* data_compile_blob(U8*blob,U32 cnt);
int data_compile_ref(U8* target);
int data_compile_token(HINDEX h);

U8* data_compile_from_file(FILE* f,U32 cnt);
// compile a ref-style <token><ref> sequence.
int data_ref_style(HINDEX htarget,char* abspath);
int data_ref_style_p(HINDEX htarget,HINDEX hoperation);
