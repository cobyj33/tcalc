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
 * Doesn't return tcalc_err, because it's just gonna sigsev if it
 * fails. I had to have done something stupid for this to actually fail like pass
 * a null pointer.
*/
size_t tcalc_strlcpy(char *dst, const char *src, size_t dsize);
tcalc_err tcalc_strdup(const char *src, char** out);

/**
 * Note that the combined string will be allocated and assigned to *out.
*/
tcalc_err tcalc_strcombine(const char *first, const char *second, char** out);

int tcalc_strisint(const char*);
tcalc_err tcalc_strtoint(const char*, int*);

int tcalc_strisdouble(const char*);
tcalc_err tcalc_strtodouble(const char*, double*);

tcalc_err find_in_strarr(const char**, size_t, const char*, size_t*);
int has_in_strarr(const char**, size_t, const char*);

tcalc_err tcalc_strsplit(const char*, char, char***, size_t* return_size);

int tcalc_streq(const char*, const char*);

int tcalc_str_list_has(const char* input, const char** list, size_t count);

/**
 * Find if one string has the prefix given by another string.
 * 
 * I use this over strncmp, as strncmp will stop when either string null terminates
 * or it reaches a maximum of the length passed. This doesn't work well for
 * prefixes though, since strncmp moreso tells that both strings share the same
 * prefix, not that one string has a certain prefix.
 * 
 * For example:
 *  strncmp("**", "**5 + 3", 2) and strncmp("**", "*", 2) eval to 0, so we can't
 *  use strncmp to find if the second string begins with "**" reliably or not  
 * 
 * Returns: 1 if str has the prefix prefix, 0 otherwise
 * 
 * prefix and str must NOT be NULL
*/
int tcalc_strhaspre(const char* prefix, const char* str);

/**
 * 
 * @param src
 * @param start (inclusive)
 * @param end (exclusive)
 * @param out the parameter at address out is allocated with the substring upon return value TCALC_OK
*/
tcalc_err tcalc_strsubstr(const char*, size_t, size_t, char**);
#endif