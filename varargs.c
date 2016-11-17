#include <stdarg.h>
#include "syscalls.h"
#include "femtolibc.h"


void _start() {
	
	int bytes_written = varargstest(5, 
			  "string 1: lalala",
			  "string 2: muahahahah",
			  "string 3: brutta carettina",
			  "string 4: cosama"
			  "string 5: lalalala\n\n\n\n"
	);
	
}

int varargstest(int n, ...) {
	va_list args;
	int total = 0;
	va_start(args, n);
	int i;
	for (i = 0; i < n; i++) {
		const char *s = va_arg(args, const char*);
		int l = strlen(s);
		if (l > 0)
			write(1, s, strlen(s));
		total += l;
	}
	va_end(args);
	return total;
}
