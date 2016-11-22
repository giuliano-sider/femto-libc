#include <stdarg.h>

#include "femtolibc.h"


static int varargstest(int n, ...);
static int printftest();

void _start() {
	
	printf("total bytes written to standard output: %d\n", printftest());
	while (1) { // 
		char c = getchar();
		printf("we are not accepting input today. here is your char back: %c\n", c);
	}
	varargstest(1, "done");	
}

static int printftest() {
	int count = 0, val = 0;
	if ((val = printf("here is a string:\n\t%s\n", "nel mezzo del cammin c'era una pietra")) < 0)
		return val; // error
	count += val;
	if ((val = printf("here's a character: %c, and another: %c and even a percent: %%\n", 'a', 'Z')) < 0)
		return val; // error
	count += val;
	if ((val = printf("this is the address of _start: %p\n", (void*)&_start)) < 0)
		return val; // error
	count += val;
	if ((val = printf("this is the address of _start: %x\n", (unsigned int)&_start)) < 0)
		return val; // error
	count += val;
	if ((val = printf("this is the address of _start: %X\n", (unsigned int)&_start)) < 0)
		return val; // error
	count += val;
	if ((val = printf("this is the address of _start: %08x\n", (unsigned int)&_start)) < 0)
		return val; // error
	count += val;
	if ((val = printf("this is the address of _start: %#08x\n", (unsigned int)&_start)) < 0)
		return val; // error
	count += val;
	if ((val = printf("this is the address of _start: %-30x\n", (unsigned int)&_start)) < 0)
		return val; // error
	count += val;
	if ((val = printf("this is the address of _start: %030x\n", (unsigned int)&_start)) < 0)
		return val; // error
	count += val;
	if ((val = printf("some numbers: %d %i %o %x %X %u\n", -2, -2, -2, -2, -2, -2)) < 0)
		return val; // error
	count += val;
	int chars;
	if ((val = printf("number of characters printed so far in this string: %n%d\n", &chars, chars)) < 0)
		return val; // error
	count += val;

	return count;
}



static int varargstest(int n, ...) {
	va_list args;
	int total = 0;
	va_start(args, n);
	int i;
	for (i = 0; i < n; i++) {
		const char *s = va_arg(args, const char*);
		int l = strlen(s);
		if (l > 0)
			write(1, s, l);
		total += l;
	}
	va_end(args);
	return total;
}
