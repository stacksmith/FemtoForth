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

$(APP): main.o  header.o data.o table.o interpret.o bindings.o src.o
	$(CPP) $(LDFLAGS) -o $(APP) main.o  header.o data.o table.o interpret.o bindings.o src.o

kernel: kernel.asm
	bin/fasmarm -s kernel.dat kernel.asm
	bin/fasmlist kernel.dat kernel.lst
	adb push kernel.bin /data/tmp
	
bindings.o: bindings.asm
	bin/fasmarm -s bindings.dat bindings.asm  bindings.o
	bin/fasmlist bindings.dat bindings.lst
	@rm -f bindings.dat 
	
main.o: main.c global.h data.h
	$(CC) -c $(INCLUDE) $(CFLAGS) main.c -o main.o 

header.o: header.c header.h global.h 
	$(CC) -c $(INCLUDE) $(CFLAGS) header.c -o header.o 

data.o: data.c data.h global.h
	$(CC) -c $(INCLUDE) $(CFLAGS) data.c -o data.o 

table.o: table.c table.h global.h
	$(CC) -c $(INCLUDE) $(CFLAGS) table.c -o table.o 
	
interpret.o: interpret.c interpret.h global.h
	$(CC) -c $(INCLUDE) $(CFLAGS) interpret.c -o interpret.o 

src.o: src.c src.h global.h
	$(CC) -c $(INCLUDE) $(CFLAGS) src.c -o src.o 

clean:
	@rm -f $(APP).lst $(APP) *.o *.lst *.dat
	
install: $(APP) kernel
	adb push $(APP) /data/tmp	
	
run: $(APP) kernel.bin
	make install
	adb shell /data/tmp/$(APP)
