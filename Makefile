#PLATFORM := android_arm
PLATFORM := linux_x86
#===============================================
ifeq ($(PLATFORM),android_arm)
APP := main
BIN := /home/miner/android-ndk-r9d/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86_64/bin
LIB := /home/miner/android-ndk-r9d/platforms/android-19/arch-arm/usr/lib
INC := /home/miner/android-ndk-r9d/platforms/android-19/arch-arm/usr/include
CPP := $(BIN)/arm-linux-androideabi-g++
CC := $(BIN)/arm-linux-androideabi-gcc
FASM := bin/fasmarm

LDFLAGS := -Wl,--entry=main,-rpath-link=$(LIB) -L$(LIB) -nostdlib -lc 
CFLAGS := -fno-short-enums -I$(INC)
endif
#===============================================
ifeq ($(PLATFORM),linux_x86)
# A simple makefile to build an android linux binary
# using FASMARM. 
CPP := g++
CC := gcc
INC := 
LIB := 
FASM := bin/fasm
#
LDFLAGS := -Wl,--entry=main,-rpath-link=$(LIB) -L$(LIB) -nostdlib -lc 
CFLAGS := -fno-short-enums -I$(INC)
endif


default: install

$(APP): main.o  header.o data.o table.o interpret.o bindings.o src.o cmd.o lang.o
	$(CPP) $(LDFLAGS) -o $(APP) main.o  header.o data.o table.o interpret.o bindings.o src.o cmd.o lang.o

kernel: $(PLATFORM)/kernel.asm
	$(FASM) -s kernel.dat $(PLATFORM)/kernel.asm
	bin/fasmlist kernel.dat kernel.lst
	adb push kernel.bin /data/tmp
	
bindings.o: $(PLATFORM)/bindings.asm
	$(FASM) -s bindings.dat $(PLATFORM)/bindings.asm  bindings.o
	bin/fasmlist bindings.dat bindings.lst
	@rm -f bindings.dat 
	
main.o: main.c global.h header.h data.h src.h
	$(CC) -c $(INCLUDE) $(CFLAGS) main.c -o main.o 

header.o: header.c global.h header.h data.h
	$(CC) -c $(INCLUDE) $(CFLAGS) header.c -o header.o 

data.o: data.c data.h global.h table.h header.h
	$(CC) -c $(INCLUDE) $(CFLAGS) data.c -o data.o 

table.o: table.c table.h global.h
	$(CC) -c $(INCLUDE) $(CFLAGS) table.c -o table.o 
	
interpret.o: interpret.c global.h header.h src.h interpret.h cmd.h table.h lang.h
	$(CC) -c $(INCLUDE) $(CFLAGS) interpret.c -o interpret.o 

src.o: src.c src.h global.h
	$(CC) -c $(INCLUDE) $(CFLAGS) src.c -o src.o 
	
cmd.o: cmd.c global.h src.h cmd.h table.h
	$(CC) -c $(INCLUDE) $(CFLAGS) cmd.c -o cmd.o 

lang.o: lang.c global.h header.h src.h interpret.h cmd.h
	$(CC) -c $(INCLUDE) $(CFLAGS) lang.c -o lang.o 
	
clean:
	@rm -f $(APP).lst $(APP) *.o *.lst *.dat *.bin	
	
#===============================================
ifeq ($(PLATFORM),android_arm)
install: $(APP) kernel
	adb push $(APP) /data/tmp	
	
run: $(APP) kernel.bin
	make install
	adb shell /data/tmp/$(APP)
endif
#===============================================
ifeq ($(PLATFORM),linux_x86)
install: $(APP) kernel
	
	
run: $(APP) kernel.bin
	make install
	
endif