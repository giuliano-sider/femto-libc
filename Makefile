
TEXTADDR = 0x77802000
DATAADDR = 0x77802C00


all: disk.img

run: disk.img
	$(ARMSIM) --rom=$(DUMBOOT) --sd=disk.img $(TAIL)

disk.img: varargs-test.elf
	$(MKSD) --so $(OS) --user $^
varargs-test.elf: varargs.o femtolibc.o syscalls.o
	$(CROSS_COMPILE)ld $^ -Ttext=$(TEXTADDR) -Tdata=$(DATAADDR) -o $@ -g
varargs.o: varargs.c femtolibc.h syscalls.h
	$(CROSS_COMPILE)gcc -c $< -o $@ -g -Wall
femtolibc.o: femtolibc.c femtolibc.h
	$(CROSS_COMPILE)gcc -c $< -o $@ -g -Wall
syscalls.o: syscalls.s
	$(CROSS_COMPILE)as $^ -o $@ -g -Wall
	
.PHONY: clean
clean:
	rm -f disk.img *.elf *.o


