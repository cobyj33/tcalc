#ifndef TCALC_STRING_H 
#define TCALC_STRING_H

#include <stddef.h>
#include "tcalc_error.h"

/**
 * Taken from FreeBSD :)
 * Copy string src to buffer dst of size dsize.  At most dsize-1
 * chars will be copied.  Always NUL terminates (unless dsize == 0).
 * Returns strlen(src); if retval >= dsize, truncation occurred.
*/
size_t tcalc_strlcpy(char *dst, const char *src, size_t dsize);
char* tcalc_strdup(const char *src);
char* tcalc_strcombine(const char *first, const char *second);

int tcalc_strisint(const char*);
tcalc_error_t tcalc_strtoint(const char*, int*);

int tcalc_strisdouble(const char*);
tcalc_error_t tcalc_strtodouble(const char*, double*);

tcalc_error_t find_in_strarr(const char**, size_t, const char*, size_t*);
int has_in_strarr(const char**, size_t, const char*);

tcalc_error_t tcalc_strsplit(const char*, char, char***, size_t* return_size);

/**
 * 
 * @param src
 * @param start (inclusive)
 * @param end (exclusive)
 * @param out (exclusive)
*/
tcalc_error_t tcalc_strsubstr(const char*, size_t, size_t, char**);
#endif