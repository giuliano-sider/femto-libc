#include <stdarg.h> // va_list, va_start, va_arg, va_end
#include "syscalls.h"


int strlen(const char *s) {
	int l = 0;
	while (s[l] != '\0')
		l++;
	return l;
}

int isspace(int c) {
	return c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

// issues: does not treat underflow/overflow
//         handles base values from 2 to 36, and 
long int strtol(const char *nptr, char **endptr, int base) {
	long int number = 0, sign = 1; // we return 0 if there are no digits
	while (*nptr != '\0' && isspace(*nptr)) // we are allowed to have leading spaces
		nptr++;
	if (*nptr != '\0') {
		if (*nptr == '+') { // a leading + or - is allowed
			++nptr;
		} else if (*nptr == '-') {
			sign = -1;
			++nptr;
		}
		if ((base == 0 || base == 16) && *nptr != '\0' && *nptr == '0' && *(nptr+1) != '\0' && (*(nptr+1) == 'x' *(nptr+1) == 'X')) {
			base = 16; // use base 16 when base is 0 and string preceded by 0x or 0X
			nptr += 2;
		} else if (base == 0 && *nptr != '\0' && *nptr == '0') {
			base = 8; // use base 8 when base is 0 and string preceded by 0
			++nptr;
		} else if (base == 0 && *nptr != '\0' && isdigit(*nptr)) {
			base = 10;
		}
		if (base < 2 || base > 36)
			return 0; // ERROR. set some errno ??
		while (isdigit(*nptr) || isalpha(*nptr)) {
			int n = *nptr;
			if (isalpha(*nptr)) { // convert {A,a,B,b, ... Z,z} to the corresponding number
				n = 10 + toupper(*nptr) - 'A';
			}
			if (n >= base)
				break;
			number *= base;
			number += n;
			++nptr;
		}
	}
	*endptr = nptr;
	return number * sign;
}


// issues: uses a 4096 byte internal buffer to avoid repeated calls to write
//         uses a couple of macros (not ideal, but allows us to use internal functions in C/portable assembler) to avoid obscene code duplication and an even messier design. alternative would be to use a 'state machine type' struct with the information pertaining to the parsing/buffering/printing, and to pass around that struct to different functions (with internal knowledge of printf)
//         uses negative return values to signal different fatal errors
int printf(const char *fmt, ...) {
	
#define PRINTFBUFFSIZE 4096
#define STDOUTWRITEERROR -1
#define INCOMPLETESPECIFIERERROR -2

// flushes the printf internal buffer to stdout
#define flushprintfbuffer() { \
	int writecount = 0; \
	while (writecount < PRINTFBUFFSIZE) { /* make sure all bytes are actually written to stdout */ \
		int r = write(1, buff, PRINTFBUFFSIZE - writecount); \
		if (r < 0) /* write error! */ \
			return STDOUTWRITEERROR; \
		writecount += r; \
	} \
	charswritten += PRINTFBUFFSIZE; \
	buffcount = 0; \
}
// end flushprintfbuffer

// writes a character to the internal buffer (flushing it if necessary).
#define addchartoprintfbuffer(c) { \
	if (buffcount == PRINTFBUFFSIZE) { \
		flushprintfbuffer(); \
	} \
	buff[buffcount] = (c); \
	buffcount++; \
}
// end addchartoprintfbuffer

	// begin printf body:
	
	va_list args;
	int charswritten = 0, buffcount = 0;
	int left_justify = 0, mustshowsign = 0, blankspaceinplaceofsign = 0, show0xor0 = 0, leftpadwithzeroes = 0, uppercasehexinteger = 0;
	int minfieldwidth = 0;
	enum {NONE, HH, H, L, LL} length_modifier = NONE;
	char buff[PRINTFBUFFSIZE];
	char number[12]; // enough to store a number we are converting
	va_start(args, fmt);
	
	while (*fmt != '\0') { // for each character in the format string
		if (*fmt == '%') {
			++fmt; // read the percent
			while (*fmt != '\0' && isprintfflag(*fmt)) { // read the printf flags
				switch(*fmt) {
				case '-':
					left_justify = 1;
					break;
				case '+':
					mustshowsign = 1;
					break;
				case ' ':
					blankspaceinplaceofsign = 1;
					break;
				case '#':
					show0xor0 = 1;
					break;
				case '0':
					leftpadwithzeroes = 1;
					break;
				}
				++fmt; // read the flag
			}
			if (*fmt == '\0') {
				return INCOMPLETESPECIFIERERROR; // where's the format specifier ?
			}
			
			if (isdigit(*fmt)) { // min field width given as a number
				minfieldwidth = strtol(fmt, &fmt, 10);
				if (*fmt == '\0') {
					return INCOMPLETESPECIFIERERROR; // where's the rest of the format specifier
				}
			} else if (*fmt == '*') { // min field width given by an integer argument to printf
				minfieldwidth = va_arg(args, int);
				++fmt;
				if (*fmt == '\0') {
					return INCOMPLETESPECIFIERERROR; // where's the rest of the format specifier
				}
			}
			
			if (*fmt == 'h' && *(fmt+1) == 'h') {
				length_modifier = HH;
				fmt += 2;
			} else if (*fmt == 'h') {
				length_modifier = H;
				fmt++;
			} else if (*fmt == 'l' && *(fmt+1) == 'l') {
				length_modifier = LL;
				fmt += 2;
			} else if (*fmt == 'l') {
				length_modifier = L;
				fmt++;
			}
			if (*fmt == '\0') {
				return INCOMPLETESPECIFIERERROR; // where's the format specifier?
			}
			
			switch (*fmt) {
			case '\0':
				return INCOMPLETESPECIFIERERROR; // where's the format specifier?
			case 'c':
			
				break;
			case 's':
			
				break;
			case '%':
			
				break;
			case 'n':
			
				break;
			default: // must be a number
				switch (*fmt) {
				case 'd':
				case 'i':
				
				case 'X':
					uppercasehexinteger = 1;
				case 'x':
					
					
				}
			}
			
			fmt++; // read the format specifier
			// reset the printf flags for the next specifier
			left_justify = 0; mustshowsign = 0; blankspaceinplaceofsign = 0; show0xor0 = 0; leftpadwithzeroes = 0;
			minfieldwidth = 0;
			length_modifier = NONE;
			uppercasehexinteger = 0;
		} else { // just copy the character onto the output
			addchartoprintfbuffer(*fmt);
			++fmt;
		}
	}
	if (buffcount > 0) {
		flushprintfbuffer();
	}
	
	va_end(args);
	return charswritten;
}


