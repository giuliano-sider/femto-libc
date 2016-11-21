
TEXTADDR = 0x77802000
DATAADDR = 0x77804000

CC_FLAGS = -O0
#LD_FLAGS = -mcpu=arm8
#AS_FLAGS = -v -mcpu=arm8 -march=armv8

all: disk.img

run: disk.img
	$(ARMSIM) --rom=$(DUMBOOT) --sd=disk.img $(TAIL)

debug:
	$(CROSS_COMPILE)gdbtui -x dbg varargs-test.elf
	#/home/mc404/simuladorfromspecg/simulador/bin/arm-eabi-gdb -ex 'target remote localhost:5000' varargs-test.elf

disk.img: varargs-test.elf
	$(MKSD) --so $(OS) --user $^
varargs-test.elf: varargs.o printf.o femtolibc.o syscalls.o divmod.o
	$(CROSS_COMPILE)ld $^ -Ttext=$(TEXTADDR) -Tdata=$(DATAADDR) -o $@ -g $(LD_FLAGS)
varargs.o: varargs.c printf.h femtolibc.h syscalls.h
	$(CROSS_COMPILE)gcc -c $< -o $@ -g -Wall $(CC_FLAGS)
printf.o: printf.c printf.h femtolibc.h syscalls.h divmod.h
	$(CROSS_COMPILE)gcc -c $< -o $@ -g -Wall $(CC_FLAGS)
femtolibc.o: femtolibc.c femtolibc.h divmod.h
	$(CROSS_COMPILE)gcc -c $< -o $@ -g -Wall $(CC_FLAGS)
syscalls.o: syscalls.s
	$(CROSS_COMPILE)as $^ -o $@ -g $(AS_FLAGS)
divmod.o: divmod.s
	$(CROSS_COMPILE)as $^ -o $@ -g $(AS_FLAGS)
	
.PHONY: clean
clean:
	rm -f disk.img *.elf *.o


