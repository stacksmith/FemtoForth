/******************************************************************************
Copyright 2014 Victor Yurkovsky

This file is part of the FemtoForth project.

FPGAsm is free software: you can redistribute it and/or modify
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
#include "global.h"

void color(U32 color){
    printf("\33[%dm",color);
}