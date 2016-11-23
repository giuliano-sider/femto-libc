#include <stdarg.h>

#include "femtolibc.h"


static int varargstest(int n, ...);
static int printftest();

void _start() {
	
	int returnval;
	if ((returnval = printf("total bytes written to standard output: %d\n", printftest())) < 0)
		puts("ERROR: printf returned negative value\n");

	while (1) { // hang
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
	if ((val = printf("this is the address of _start + 12: %p\n", (void*)&_start + 12)) < 0)
		return val; // error
	count += val;
	if ((val = printf("this is the address of _start + 12: %x\n", (unsigned int)&_start + 12)) < 0)
		return val; // error
	count += val;
	if ((val = printf("this is the address of _start + 12: %X\n", (unsigned int)&_start + 12)) < 0)
		return val; // error
	count += val;
	if ((val = printf("this is the address of _start + 12: %010x\n", (unsigned int)&_start + 12)) < 0)
		return val; // error
	count += val;
	if ((val = printf("this is the address of _start + 12: %#08x\n", (unsigned int)&_start + 12)) < 0)
		return val; // error
	count += val;
	if ((val = printf("this is the address of _start + 12: %- 030x\n", (unsigned int)&_start + 12)) < 0)
		return val; // error
	count += val;
	if ((val = printf("this is the address of _start + 12: %+030x\n", (unsigned int)&_start + 12)) < 0)
		return val; // error
	count += val;
	if ((val = printf("some numbers: %d %i %o %x %X %u\n", -2, -2, -2, -2, -2, -2)) < 0)
		return val; // error
	count += val;
	int chars = 989;
	if ((val = printf("number of characters printed so far in this string: %n(this string has some more characters too)\n", &chars)) < 0)
		return val; // error
	count += val;
	if ((val = printf("%d\n", chars)) < 0)
		return val; // error
	count += val;
	if ((val = printf("some numbers: %i %o %x %c %X\n", 1234, 1234, 1234, 1234, 1234)) < 0)
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
