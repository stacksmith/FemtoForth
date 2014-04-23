# A simple makefile to build an android linux binary
# using FASMARM. 
APP := main
BIN := /home/miner/android-ndk-r9d/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86_64/bin
LIB := /home/miner/android-ndk-r9d/platforms/android-19/arch-arm/usr/lib
INC := /home/miner/android-ndk-r9d/platforms/android-19/arch-arm/usr/include
CPP := $(BIN)/arm-linux-androideabi-g++
CC := $(BIN)/arm-linux-androideabi-gcc

LDFLAGS := -Wl,--entry=main,-rpath-link=$(LIB) -L$(LIB) -nostdlib -lc 
CFLAGS := -fno-short-enums -I$(INC)

default: install

$(APP): main.o hello.o header.o data.o table.o interpret.o
	$(CPP) $(LDFLAGS) -o $(APP) main.o hello.o header.o data.o table.o interpret.o

kernel: kernel.asm
	bin/fasmarm -s kernel.dat kernel.asm
	bin/fasmlist kernel.dat kernel.lst
	adb push kernel.bin /data/tmp
	
hello.o: hello.asm
	bin/fasmarm -s hello.dat hello.asm
	bin/fasmlist hello.dat hello.lst
	@rm -f hello.dat 
	
main.o: main.c global.h data.h
	$(CC) -c $(INCLUDE) $(CFLAGS) main.c -o main.o 

header.o: header.c header.h global.h 
	$(CC) -c $(INCLUDE) $(CFLAGS) header.c -o header.o 

data.o: data.c data.h global.h
	$(CC) -c $(INCLUDE) $(CFLAGS) data.c -o data.o 

table.o: table.c table.h global.h
	$(CC) -c $(INCLUDE) $(CFLAGS) table.c -o table.o 
	
interpret.o: interpret.c interpret.h global.h
clean:
	@rm -f $(APP).lst $(APP) *.o *.lst *.dat
	
install: $(APP) kernel
	adb push $(APP) /data/tmp	
	
run: $(APP) kernel.bin
	make install
	adb shell /data/tmp/$(APP)
