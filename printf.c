// *** femtolibc printf implementation ***
// issues: uses a 4096 byte internal (stack allocated) buffer to avoid repeated calls to write
//         uses negative return values to signal different fatal errors
//         doesn't really check for any ridiculous user inputs, like left-padding a string with zeroes...
//         the length modifier field ends up not really being used except for the case of %n (va_arg does default promotion to int or unsigned int, and the long, etc. are the same size as int in this arch)
//         no floating point support. no gnu glibc specific extensions.
//         see `man 3 printf` for more details

#include "printf.h"




// flushes the printf internal buffer to stdout. returns number of characters written
static int flushprintfbuffer(struct printf_state_machine *psm) {
	int writecount = 0;
	while (psm->buffcount > 0) { // make sure all bytes are actually written to stdout 
		int r = write(STDOUT, psm->buff + writecount, psm->buffcount);
		if (r < 0) { // write error!
			psm->error_indicator = STDOUTWRITEERROR;
			return r;
		}
		writecount += r;
		psm->buffcount -= r;
	}
	psm->charswritten += writecount;
	return writecount;
}

// writes a character to the internal buffer (flushing it if necessary).
static void addchartoprintfbuffer(struct printf_state_machine *psm, char c) {
	if (psm->buffcount == PRINTFBUFFSIZE) {
		flushprintfbuffer(psm);
	}
	psm->buff[psm->buffcount] = c;
	psm->buffcount++;
}


static void print_field(struct printf_state_machine *psm, char *field) {
				
	int l = strlen(field);

	if (psm->flags.show0xor0 && !(l == 1 && field[0] == '0')) {
		if (psm->specifier == 'o') {
			field[l] = '0';
			l++;
		} else if (psm->specifier == 'x' || psm->specifier == 'X') {
			field[l] = psm->specifier;
			field[l+1] = '0';
			l += 2;
		}
	}
	debugprint("print_field 1")
	if (psm->flags.mustshowsign || psm->sign == '-') {
		field[l] = psm->sign;
		l++;
	} else if (psm->flags.blankspaceinplaceofsign) {
		field[l] = ' ';
		l++;
	}
	debugprint("print_field 2")
	field[l] = '\0';
	if (psm->specifier == 'x' || psm->specifier == 'X' || psm->specifier == 'o' || psm->specifier == 'd' || psm->specifier == 'i')
		strrev(field); // numbers are backwards
	debugprint("print_field 3")
	int padding = psm->minfieldwidth - l;
	if (psm->flags.left_justify) {
		int i;
		for (i = 0; i < l; i++) {
			addchartoprintfbuffer(psm, field[i]);
		}
		while (padding > 0) {
			addchartoprintfbuffer(psm, psm->flags.leftpadwithzeroes ? '0' : ' ');
			padding--;
		}
	} else { // default: field is right justified within the field width
		while (padding > 0) {
			addchartoprintfbuffer(psm, psm->flags.leftpadwithzeroes ? '0' : ' ');
			padding--;
		}
		int i;
		for (i = 0; i < l; i++) {
			addchartoprintfbuffer(psm, field[i]);
		}
	}
	debugprint("print_field 4")
}

static void reset_specifier_read(struct printf_state_machine *psm) {
	psm->readingstate = READNEXTCHAR;
	psm->flags.left_justify = 0;
	psm->flags.mustshowsign = 0;
	psm->flags.blankspaceinplaceofsign = 0;
	psm->flags.show0xor0 = 0;
	psm->flags.leftpadwithzeroes = 0;
	psm->minfieldwidth = 0;
	psm->length_modifier = NONE;
	psm->sign = '+';
}

static void init_printf_state_machine(const char *fmt, struct printf_state_machine *psm) {
	psm->readingstate = READNEXTCHAR;
	psm->readingpos = fmt;
	psm->currentchar = *(psm->readingpos);
	psm->charswritten = 0;
	psm->buffcount = 0;
	psm->flags.left_justify = 0;
	psm->flags.mustshowsign = 0;
	psm->flags.show0xor0 = 0;
	psm->flags.leftpadwithzeroes = 0;
	psm->minfieldwidth = 0; // means there is no minimum field width
	psm->length_modifier = NONE;





	psm->sign = '+';
	psm->error_indicator = 0;
}

//int hasnextchar(struct printf_state_machine *psm) {
//	return *(psm->readingpos) != '\0';
//}

char readnextchar(struct printf_state_machine *psm) {
	psm->readingpos++;
	psm->currentchar = *(psm->readingpos);
	return psm->currentchar;
}
	

int printf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int writecount = vprintf(fmt, args);
	va_end(args);
	return writecount;
}

int vprintf(const char *fmt, va_list args) {

	struct printf_state_machine psm; // by allocating it on the stack, we avoid race conditions
	init_printf_state_machine(fmt, &psm);

	while (psm.currentchar != '\0') { // while we still have characters in the format string to read, do:
		
		switch (psm.readingstate) {
		case READNEXTCHAR:
			if (psm.currentchar == '%') {
				psm.readingstate = READFLAGS;
			} else {
				addchartoprintfbuffer(&psm, psm.currentchar);
			}
			readnextchar(&psm);
			break;
		case READFLAGS:
			switch (psm.currentchar) {
			case '-':
				psm.flags.left_justify = 1;
				readnextchar(&psm);
				break;
			case '+':
				psm.flags.mustshowsign = 1;
				readnextchar(&psm);
				break;
			case ' ':
				psm.flags.blankspaceinplaceofsign = 1;
				readnextchar(&psm);
				break;
			case '#':
				psm.flags.show0xor0 = 1;
				readnextchar(&psm);
				break;
			case '0':
				psm.flags.leftpadwithzeroes = 1;
				readnextchar(&psm);
				break;
			default:
				psm.readingstate = READWIDTH;
				break;
			}
			break;
		case READWIDTH:
			if (psm.currentchar == '*') {
				psm.minfieldwidth = va_arg(args, int); // read the minimum field width from the next printf argument
				readnextchar(&psm);
			} else if (isdigit(psm.currentchar)) {
				psm.minfieldwidth = strtol(psm.readingpos, (char**)&psm.readingpos, 10); // read number representing the width
			} else {
				psm.readingstate = READLENGTH;
			}
			break;
		case READLENGTH: // length modifiers are 'hh', 'h', 'l', 'll'
			if (psm.currentchar == 'h') {
				readnextchar(&psm);
				if (psm.currentchar == 'h') { // hh
					psm.length_modifier = HH;
					readnextchar(&psm);
				} else { // h
					psm.length_modifier = H;
				}
			} else if (psm.currentchar == 'l') {
				readnextchar(&psm);
				if (psm.currentchar == 'l') { // ll
					psm.length_modifier = LL;
					readnextchar(&psm);
				} else { // l
					psm.length_modifier = L;
				}
			}
			psm.readingstate = READSPECIFIER;
			break;
		case READSPECIFIER:
			psm.specifier = psm.currentchar;
			switch (psm.specifier) {
			case 'c':
				psm.asciinumberfield[0] = (char)va_arg(args, int);
				psm.asciinumberfield[1] = '\0';
				print_field(&psm, psm.asciinumberfield);
				break;
			case 's':
				print_field(&psm, va_arg(args, char *));
				break;
			case 'n':
				switch (psm.length_modifier) {
				case NONE:
					*va_arg(args, int*) = psm.charswritten + psm.buffcount;
					break;
				case HH:
					*va_arg(args, signed char*) = psm.charswritten + psm.buffcount;
					break;
				case H:
					*va_arg(args, short int*) = psm.charswritten + psm.buffcount;
					break;
				case L:
					*va_arg(args, long int*) = psm.charswritten + psm.buffcount;
					break;
				case LL:
					*va_arg(args, long long int*) = psm.charswritten + psm.buffcount;
					break;
				}
				break;
			case '%':
				psm.asciinumberfield[0] = '%';
				psm.asciinumberfield[1] = '\0';
				print_field(&psm, psm.asciinumberfield);
				break;
			case 'd':
			case 'i':
				;int number = va_arg(args, int);
				if (number < 0) {
					psm.sign = '-';
					number = -number;
				}
				uitoa(number, psm.asciinumberfield, 10);
				print_field(&psm, psm.asciinumberfield);
				break;
			case 'u':
				uitoa(va_arg(args, unsigned int), psm.asciinumberfield, 10);
				print_field(&psm, psm.asciinumberfield);
				break;
			case 'o':
				uitoa(va_arg(args, unsigned int), psm.asciinumberfield, 8);
				print_field(&psm, psm.asciinumberfield);
				break;
			case 'p': // issue: we cast the pointer to an unsigned int, which works on this specific 32 bit architecture
				debugprint("in specifier p handler, before utioa");
				uitoa((unsigned int)va_arg(args, void*), psm.asciinumberfield, 16);
				puts("in specifier p handler, after uitoa");
				psm.flags.show0xor0 = 1;
				psm.specifier = 'x';
				print_field(&psm, psm.asciinumberfield);
				puts("in specifier p handler, after print_field");
				break;
			case 'X':
			case 'x':
				uitoa(va_arg(args, unsigned int), psm.asciinumberfield, 16);
				if (psm.currentchar == 'X')
					strtoupper(psm.asciinumberfield);
				print_field(&psm, psm.asciinumberfield);
				break;
			default:
				psm.error_indicator = UNKNOWNFORMATSPECIFIER;
				return psm.error_indicator;
			} // end switch (specifier)
			readnextchar(&psm);
			reset_specifier_read(&psm); // go back to reading regular character sequences
			break;
		} // end switch (curent character)
		
	} // while there are still characters to read from the format string
	
	if (psm.readingstate != READNEXTCHAR) { // error: in the middle of an incomplete format specifier
		psm.error_indicator = INCOMPLETESPECIFIERERROR;
	}

	// whatever is left in the buffer must be flushed
	flushprintfbuffer(&psm);
	
	if (psm.error_indicator == 0)
		return psm.charswritten; // return total number of characters written
	else
		return psm.error_indicator; // (negative) error indicator returned
}


