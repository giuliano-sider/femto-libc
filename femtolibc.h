#ifndef FEMTO_LIBC_H
#define FEMTO_LIBC_H

#define NULL 0
#define EOF -1

#define STDIN 0
#define STDOUT 1

#include "syscalls.h"
#include "divmod.h" // gcc doesn't compile division and modulo operations
#include "printf.h"
#include "divmod.h"


// debug time macros

#ifndef SIGNOFFALLFUNCTIONS
#define SIGN(s) puts(s);
#else
#define SIGN(s) 
#endif

#define NDEBUG
// arm-eabi-gdbtui is behaving badly: it does a 'continue' instead of a 'next' or 'step' in many situations
#ifndef NDEBUG
#define debugprint(s) puts(s);
#else 
#define debugprint(s)
#endif


int strlen(const char *s);
int isspace(int c);
int isdigit(int c);
int isalpha(int c);
int toupper(int c);
void *memset(void *buff, int byte, int n);
char *strrev(char *s);
int getchar();
long int strtol(const char *nptr, char **endptr, int base);
int uitoa(unsigned int number, char *buff, int radix);
void strtoupper(char *s);
int puts(const char *s);

#endif // FEMTO_LIBC_H
