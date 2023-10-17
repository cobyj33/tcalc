#ifndef TCALC_STRING_H 
#define TCALC_STRING_H

#include <stddef.h>
#include "tcalc_error.h"

/**
 * Taken from FreeBSD :)
 * Copy string src to buffer dst of size dsize.  At most dsize-1
 * chars will be copied.  Always NUL terminates (unless dsize == 0).
 * Returns strlen(src); if retval >= dsize, truncation occurred.
 * 
 * "Note that room for the NUL should be included in [dsize]."
 *  - FreeBSD Library Functions Manual (https://man.freebsd.org/cgi/man.cgi?query=strlcpy&sektion=3)
 * 
 * Doesn't return tcalc_error_t, because it's just gonna sigsev if it
 * fails. I had to have done something stupid for this to actually fail like pass
 * a null pointer.
*/
size_t tcalc_strlcpy(char *dst, const char *src, size_t dsize);
tcalc_error_t tcalc_strdup(const char *src, char** out);

/**
 * Note that the combined string will be allocated and assigned to *out.
*/
tcalc_error_t tcalc_strcombine(const char *first, const char *second, char** out);

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
 * @param out the parameter at address out is allocated with the substring upon return value TCALC_OK
*/
tcalc_error_t tcalc_strsubstr(const char*, size_t, size_t, char**);
#endif