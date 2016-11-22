#ifndef PRINTF_H
#define PRINTF_H

#include <stdarg.h>

#include "femtolibc.h"

int printf(const char *fmt, ...);
int vprintf(const char *fmt, va_list args);
	
#define PRINTFBUFFSIZE 256
#define STDOUTWRITEERROR -1
#define INCOMPLETESPECIFIERERROR -2
#define UNKNOWNFORMATSPECIFIER -3


struct printf_state_machine {
	enum printf_machine_state {
		READNEXTCHAR,
		READFLAGS,
		READWIDTH,
		READLENGTH,
		READSPECIFIER
	} readingstate; // state of the format string reading machine
	const char *readingpos; // points to the next character to be read
	char currentchar; // the current character our machine is examining
	int charswritten; // number of characters already written to stdout
	int buffcount; // number of characters currently in the buffer
	struct printf_flags {
		char left_justify; // left justifies the printed field within the minimum field width
		char mustshowsign; // shows the sign of a number (even if positive)
		char blankspaceinplaceofsign; // if previous flag is clear and number is +, prints a space in place of +
		char show0xor0; // if appropriate, precede number with 0, 0x, or 0X
		char leftpadwithzeroes; // left pad with zeroes instead of spaces
	} flags;
	int minfieldwidth; // at least this many characters have to be printed
	enum length_modifier {
		NONE, HH, H, L, LL
	} length_modifier;
	char buff[PRINTFBUFFSIZE]; // internal (stack allocated) buffer for storing characters before they are sent to stdout
	char asciinumberfield[16]; // enough to store a number we are converting to characters
	char specifier; // format specifier: %, c, d, i, n, o, p, x, X, u 
	//enum number_type {
	//	uc,
	//	sc,
	//	si,
	//	usi,
	//	i,
	//	ui,
	//	li,
	//	uli,
	//	lli
	//} number_type;
	//union number {
	//	unsigned char uc;
	//	signed char sc;
	//	short int si;
	//	unsigned short int usi;
	//	int i;
	//	unsigned int ui;
	//	long int li;
	//	unsigned long int uli;
	//	long long int lli;
	//} number;
	char sign; // sign of number to be printed
	int error_indicator;
};



#endif // PRINTF_H