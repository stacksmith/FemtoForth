#******************************************************************************
# Copyright 2014 Victor Yurkovsky
#
# This file is part of the FemtoForth project.
#
# FemtoForth is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# FemtoForth is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY# without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with FemtoForth. If not, see <http://www.gnu.org/licenses/>.
#
#*****************************************************************************
# uncomment one of the supported platforms
#PLATFORM := android_arm
PLATFORM := linux_x86
#===============================================
ifeq ($(PLATFORM),android_arm)
BIN := /home/miner/android-ndk-r9d/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86_64/bin
LIB := /home/miner/android-ndk-r9d/platforms/android-19/arch-arm/usr/lib
INC := /home/miner/android-ndk-r9d/platforms/android-19/arch-arm/usr/include
CPP := $(BIN)/arm-linux-androideabi-g++
CC := $(BIN)/arm-linux-androideabi-gcc
FASM := ../tools/fasmarm

LDFLAGS := -Wl,--entry=main,-rpath-link=$(LIB) -L$(LIB) -nostdlib -lc 
CFLAGS := -fno-short-enums -I$(INC)
default: install
endif
#===============================================
ifeq ($(PLATFORM),linux_x86)
# A simple makefile to build an android linux binary
# using FASMARM. 
CPP := gcc
CC := gcc
INC :=  /usr/include
LIB :=  /usr/lib32
FASM := ../tools/fasm
#
LDFLAGS := -m32 -L$(LIB)  -lc 
CFLAGS := -m32 -fno-short-enums -I$(INC)
default: main
endif

APP := main


$(APP):  main.o  header.o data.o table.o interpret.o bindings.o src.o cmd.o lang.o color.o kernel.bin
	$(CPP) $(LDFLAGS)  -o $(APP) main.o header.o data.o table.o interpret.o bindings.o src.o cmd.o lang.o color.o

kernel.bin: platform/$(PLATFORM)/kernel.asm platform/kernel_common.s
	$(FASM) -s kernel.dat platform/$(PLATFORM)/kernel.asm kernel.bin
	../tools/fasmlist kernel.dat kernel.lst
ifeq ($(PLATFORM),android_arm)
	adb push kernel.bin /data/tmp
endif
	@rm -f kernel.dat 
	
bindings.o: platform/$(PLATFORM)/bindings.asm
	$(FASM) -s bindings.dat platform/$(PLATFORM)/bindings.asm  bindings.o
	../tools/fasmlist bindings.dat bindings.lst
	@rm -f bindings.dat 
	
main.o: main.c global.h header.h data.h src.h lang.h color.h
	$(CC) -c $(INCLUDE) $(CFLAGS) main.c -o main.o 

header.o: header.c global.h header.h data.h
	$(CC) -c $(INCLUDE) $(CFLAGS) header.c -o header.o 

data.o: data.c data.h global.h table.h header.h 
	$(CC) -c $(INCLUDE) $(CFLAGS) data.c -o data.o 

table.o: table.c table.h global.h header.h
	$(CC) -c $(INCLUDE) $(CFLAGS) table.c -o table.o 
	
interpret.o: interpret.c global.h header.h src.h interpret.h cmd.h table.h lang.h data.h color.h
	$(CC) -c $(INCLUDE) $(CFLAGS) interpret.c -o interpret.o 

src.o: src.c src.h global.h
	$(CC) -c $(INCLUDE) $(CFLAGS) src.c -o src.o 
	
cmd.o: cmd.c global.h src.h cmd.h table.h lang.h
	$(CC) -c $(INCLUDE) $(CFLAGS) cmd.c -o cmd.o 

lang.o: lang.c lang.h global.h header.h src.h interpret.h cmd.h data.h table.h
	$(CC) -c $(INCLUDE) $(CFLAGS) lang.c -o lang.o 

color.o: color.c color.h global.h
	$(CC) -c $(INCLUDE) $(CFLAGS) color.c -o color.o 

clean:
	@rm -f $(APP).lst $(APP) *.o *.lst *.dat *.bin	
	
#===============================================
ifeq ($(PLATFORM),android_arm)
install: $(APP) kernel.bin
	adb push $(APP) /data/tmp	
	
run: $(APP) kernel.bin
	make install
	adb shell /data/tmp/$(APP)
endif
#===============================================
ifeq ($(PLATFORM),linux_x86)
install: $(APP) kernel.bin
	
	
	
run: $(APP) kernel.bin
	./$(APP)
	
endif