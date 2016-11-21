#include <stdarg.h> // va_list, va_start, va_arg, va_end
// freestanding C compiler should have: float.h, iso646.h, limits.h, stdarg.h, stdbool.h, stddef.h, stdint.h (C99 standard 4.6)

#include "femtolibc.h"
#include "syscalls.h"
#include "divmod.h" // software division


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

void strtoupper(char *s) {
	while (*s != '\0') {
		if ('a' <= *s && *s <= 'z')
			*s -= 'a' - 'A';
		s++;
	}
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



// issues: does not treat underflow/overflow. does not set errno.
//        base is either special value 0, or between 2-36. returns 0 if base is invalid.
//        see `man strtol` for details
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
	if (endptr != NULL)
		*endptr = (char*)nptr;
	return number * sign;
}

// input: a(n unsigned) number, a buffer where to store the
// ascii representation of the number, and the radix in which
// to represent it. number of characters written to the buffer
// is returned (not including nul terminator). radix must be between 2 and 36.
int uitoa(unsigned int number, char *buff, int radix) {
	static const char digits[] = { // lookup table for converting numbers to char
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
		'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
		'u', 'v', 'w', 'x', 'y', 'z'
	};
	if (radix < 2 || radix > 36)
		return 0; // error: maybe set some kind of errno?
	int length = 0;
	if (number == 0) {
		*buff = '0';
		buff++;
		length++;
	}
	while (number > 0) {
		*buff = digits[umod(number, radix)];
		number = udiv(number, radix); // homebrew division functions
		buff++;
		length++;
	}
	*buff = '\0';
	return length;
}




