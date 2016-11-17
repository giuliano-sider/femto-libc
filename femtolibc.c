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

int isdigit(int c) {
	return c <= '9' && c >= '0';
}

int isalpha(int c) {
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

int toupper(int c) {
	return ('a' <= c && c <= 'z') ? c - ('a' - 'A') : c;
}

void *memset(void *buff, int byte, int n) {
	char *b = (char*) buff; // assuming chars are 1 byte
	int i = 0;
	while (i < n) {
		b[i] = (char)byte;
	}
	return buff;
}

char *strrev(char *s) {
	int l = strlen(s) - 1;
	int i = 0;
	while (i < l) {
		char tmp = s[l];
		s[l] = s[i];
		s[i] = tmp;
	}
	return s;
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
		if ((base == 0 || base == 16) && *nptr != '\0' && *nptr == '0' && *(nptr+1) != '\0' && (*(nptr+1) == 'x' || *(nptr+1) == 'X')) {
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
	*endptr = (char*)nptr;
	return number * sign;
}


// issues: uses a 4096 byte internal buffer to avoid repeated calls to write
//         uses a couple of macros (not ideal, but allows us to use internal functions in C/portable assembler) to avoid obscene code duplication and an even messier design. alternative would be to use a 'state machine type' struct with the information pertaining to the parsing/buffering/printing, and to pass around that struct to different functions (with internal knowledge of printf)
//         uses negative return values to signal different fatal errors
//         doesn't really check for any ridiculous user inputs, like left-padding a string with zeroes...
//         the length modifier field ends up not really being used except for the case of %n (va_arg does default promotion to int, and the long, etc. are the same size as int in this arch)
int printf(const char *fmt, ...) {
	
#define PRINTFBUFFSIZE 4096
#define STDOUTWRITEERROR -1
#define INCOMPLETESPECIFIERERROR -2
#define UNKNOWNFORMATSPECIFIER -3

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

#define isprintfflag(c) ((c) == ' ' || (c) == '+' || (c) == '-' || (c) == '#' || (c) == '0')

	// begin printf body:
	
	int charswritten = 0, buffcount = 0;
	int left_justify = 0, mustshowsign = 0, blankspaceinplaceofsign = 0, show0xor0 = 0, leftpadwithzeroes = 0;
	int minfieldwidth = 0; // 0 => no minimum field width
	int uppercasehexinteger = 0;
	enum {NONE, HH, H, L, LL} length_modifier = NONE;
	char buff[PRINTFBUFFSIZE];
	char asciinumberfield[16]; // enough to store a number we are converting
	char *fieldptr = asciinumberfield;
	int radix = 0;
	char sign = '+';
	char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	int number = 0; // holds any numeric argument (including pointers -> non portable)
	const char *emptystring = "\0";

	va_list args;
	va_start(args, fmt); // let the variadic argument extraction begin
	
	while (*fmt != '\0') { // for each character in the format string, do:
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
				minfieldwidth = strtol(fmt, (char**)&fmt, 10);
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
				asciinumberfield[0] = va_arg(args, int);
				asciinumberfield[1] = '\0';
				break;
			case 's':
				fieldptr = va_arg(args, char*);
				break;
			case '%':
				asciinumberfield[0] = '%';
				asciinumberfield[1] = '\0';
				break;
			case 'n': // number of chars written so far is written at the place pointed to by the argument
				minfieldwidth = 0;
				fieldptr = (char*)emptystring;
				if (length_modifier == NONE) {
					*(va_arg(args, int*)) = charswritten;
				} else if (length_modifier == NONE) {
					*(va_arg(args, signed char*)) = charswritten;
				} else if (length_modifier == NONE) {
					*(va_arg(args, short int*)) = charswritten;
				} else if (length_modifier == NONE) {
					*(va_arg(args, long int*)) = charswritten;
				} else if (length_modifier == NONE) {
					*(va_arg(args, long long int*)) = charswritten;
				}
				break;
			
			case 'd':
			case 'i':
				if (length_modifier == NONE) {
					number = va_arg(args, int);
				} else if (length_modifier == HH) {
					number = va_arg(args, int);
				} else if (length_modifier == H) {
					number = va_arg(args, int);
				} else if (length_modifier == L) {
					number = va_arg(args, long int);
				} else if (length_modifier == LL) {
					number = va_arg(args, long long int);
				}
				if (number < 0) {
					sign = '-';
					number *= -1;
				}
				radix = 10;
				break;
			case 'o':
				if (length_modifier == NONE) {
					number = va_arg(args, unsigned int);
				} else if (length_modifier == HH) {
					number = va_arg(args, unsigned int);
				} else if (length_modifier == H) {
					number = va_arg(args, unsigned int);
				} else if (length_modifier == L) {
					number = va_arg(args, unsigned long int);
				} else if (length_modifier == LL) {
					number = va_arg(args, unsigned long long int);
				}
				radix = 8;
				break;
			case 'p':
				number = (int)va_arg(args, void*);
				show0xor0 = 1;
				radix = 16;
				break;
			case 'u':
				if (length_modifier == NONE) {
					number = va_arg(args, unsigned int);
				} else if (length_modifier == HH) {
					number = va_arg(args, unsigned int);
				} else if (length_modifier == H) {
					number = va_arg(args, unsigned int);
				} else if (length_modifier == L) {
					number = va_arg(args, unsigned long int);
				} else if (length_modifier == LL) {
					number = va_arg(args, unsigned long long int);
				}
				radix = 10;
				break;
			case 'X':
				uppercasehexinteger = 1;
			case 'x':
				if (length_modifier == NONE) {
					number = va_arg(args, unsigned int);
				} else if (length_modifier == HH) {
					number = va_arg(args, unsigned int);
				} else if (length_modifier == H) {
					number = va_arg(args, unsigned int);
				} else if (length_modifier == L) {
					number = va_arg(args, unsigned long int);
				} else if (length_modifier == LL) {
					number = va_arg(args, unsigned long long int);
				}
				radix = 16;
				break;
			default:
				return UNKNOWNFORMATSPECIFIER;	
				
			}
			if (radix != 0) { // if we in fact have a number to convert to string (radix 0 is NaN)
				if (number == 0) {
					fieldptr[0] = '0';
				}
				int i = 0;
				while (number > 0) {
					fieldptr[i] = digits[mod(number, radix)];
					number = udiv(number, radix);
					i++;
				}
				if (show0xor0) {
					if (radix == 8) {
						fieldptr[i] = '0';
						i++;
					} else if (radix == 16) {
						fieldptr[i] = 'x';
						fieldptr[i+1] = '0';
						i += 2;
					}
				}
				if (mustshowsign) {
					fieldptr[i] = sign;
					i++;
				} else if (blankspaceinplaceofsign) {
					fieldptr[i] = ' ';
					i++;
				}
				fieldptr[i] = '\0';
				strrev(fieldptr);
				if (uppercasehexinteger) {
					char *c = fieldptr;
					while (*c != '\0') {
						*c = toupper(*c);
					}
				}
			}
			minfieldwidth -= strlen(fieldptr);
			if (left_justify) {
				while (*fieldptr != '\0') {
					addchartoprintfbuffer(*fieldptr);
					fieldptr++;
				}
				while (minfieldwidth > 0) {
					addchartoprintfbuffer(leftpadwithzeroes ? '0' : ' ');
					minfieldwidth--;
				}
			} else { // default: field is right justified within the field width
				while (minfieldwidth > 0) {
					addchartoprintfbuffer(leftpadwithzeroes ? '0' : ' ');
					minfieldwidth--;
				}
				while (*fieldptr != '\0') {
					addchartoprintfbuffer(*fieldptr);
					fieldptr++;
				}
			}
			
			fmt++; // consume the format specifier

			// reset the printf flags for the next specifier
			left_justify = 0; mustshowsign = 0; blankspaceinplaceofsign = 0; show0xor0 = 0; leftpadwithzeroes = 0;
			minfieldwidth = 0;
			length_modifier = NONE;
			uppercasehexinteger = 0;
			fieldptr = asciinumberfield;
			sign = '+';
			radix = 0;
			memset(&number, 0, sizeof(number));
		} else { // just copy a normal character onto the output
			addchartoprintfbuffer(*fmt);
			++fmt;
		}
	}
	if (buffcount > 0) { // whatever is left in the buffer must be flushed
		flushprintfbuffer();
	}
	
	va_end(args);
	return charswritten;
}


