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
//interpret.h
//for binding to meow-meow:

void interpret_init();
int interpret_compone(char* ptr,U32 cnt);
int interpret_compuntil(char* delim, U32 delimcnt);
int interpret_outer();
int interpret_one();

void dstack_push(U32 val);
U32  dstack_pop();
void dstack_write(U32 val);
U32  dstack_read();
