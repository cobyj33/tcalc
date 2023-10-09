#ifndef TCALC_DSTRING_H
#define TCALC_DSTRING_H

#include "tcalc_error.h"
#include <stddef.h>

/**
 * Dynamic string which holds c-like string data
*/
typedef struct tcalc_dstring_impl tcalc_dstring;

/**
 * Allocates and returns tcalc_dstring struct
 * 
 * Can return NULL on allocation failure
*/
tcalc_dstring* tcalc_dstring_alloc();

/**
 * Does not include final NULL byte in size count
*/
size_t tcalc_dstring_size(tcalc_dstring*);

void tcalc_dstring_free(tcalc_dstring*);

void tcalc_dstring_clear(tcalc_dstring*);


/**
 * Set the tcalc_dstring to contain the same data as the provided substring of a
 * c-like string
 * 
 * @param str The tcalc_dstring to set the string value of
 * @param src The source string to copy
 * @param start The starting index (inclusive) of the 
 * @param end The end (exclusive) of the section to copy
*/
tcalc_error_t tcalc_dstring_set_csubstr(tcalc_dstring*, const char*, size_t, size_t);

/**
 * Set the tcalc_dstring to contain the same data as the provided c-like string
 * 
 * @param str The tcalc_dstring to set the string value of
 * @param src The source string to copy
 * @param start The starting index (inclusive) of the 
 * @param end The end (exclusive) of the section to copy
*/
tcalc_error_t tcalc_dstring_set_cstr(tcalc_dstring*, const char*);

tcalc_error_t tcalc_dstring_strlcpy_get(tcalc_dstring*, char*, size_t);

tcalc_error_t tcalc_dstring_cstrdup(tcalc_dstring*, char**);

int tcalc_dstring_strcmp(tcalc_dstring*, const char*);

int tcalc_dstring_isint(tcalc_dstring*);
int tcalc_dstring_isdouble(tcalc_dstring*);

tcalc_error_t tcalc_dstring_append_cstr(tcalc_dstring*, char*);
// tcalc_error_t tcalc_dstring_append_csubstr(tcalc_dstring*, char*);

tcalc_error_t tcalc_dstring_getint(tcalc_dstring*, int*);
tcalc_error_t tcalc_dstring_getdouble(tcalc_dstring*, double*);


#endif