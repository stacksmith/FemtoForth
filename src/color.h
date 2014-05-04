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

#define FORE_BLACK 30
#define FORE_RED 31
#define FORE_GREEN 32
#define FORE_YELLOW 33
#define FORE_BLUE 34
#define FORE_MAGENTA 35
#define FORE_CYAN 36
#define FORE_WHITE 37
#define COLOR_RESET 0
#define COLOR_BRIGHT 1
#define COLOR_DIM 2

void color(U32 col);