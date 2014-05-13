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
//TINDEX table_add_ptr(U8* ptr,HINDEX h);
U8** table_base(U8* p);


TOKEN   table_find_or_create(TOKEN* address,TOKEN* target);
PTOKEN* table_end(PTOKEN address);
void    table_wipe(PTOKEN* address);
TOKEN** table_base(TOKEN* p);

int table_clean(PTOKEN* ptable);




