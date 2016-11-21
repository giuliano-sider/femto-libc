#ifndef FEMTO_LIBC_H
#define FEMTO_LIBC_H

#define NULL 0

#include "syscalls.h"
#include "divmod.h" // gcc doesn't compile division and modulo operations
#include "printf.h"
#include "divmod.h"

int strlen(const char *s);
int isspace(int c);
int isdigit(int c);
int isalpha(int c);
int toupper(int c);
void *memset(void *buff, int byte, int n);
char *strrev(char *s);
long int strtol(const char *nptr, char **endptr, int base);
int uitoa(unsigned int number, char *buff, int radix);
void strtoupper(char *s);

#endif // FEMTO_LIBC_H
