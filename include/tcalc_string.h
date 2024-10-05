#ifndef TCALC_STRING_H
#define TCALC_STRING_H

#include <stddef.h>
#include <tcalc_error.h>

typedef struct tcalc_strv {
  char* str;
  size_t len;
} tcalc_strv;

typedef struct tcalc_dstr {
  size_t len;
  size_t cap;
  char str[];
} tcalc_dstr;

typedef struct tcalc_slice {
  size_t start;
  size_t xend;
} tcalc_slice;

typedef struct tcalc_strslice {
  char* str;
  size_t start;
  size_t xend;
} tcalc_strslice;


static inline size_t tcalc_slice_len(tcalc_slice slice) {
  return slice.xend - slice.start;
}

inline struct tcalc_strv tcalc_dstr_to_strv(struct tcalc_dstr dstr) {
  return (struct tcalc_strv){ .str = dstr.str, .len = dstr.len };
}

#define TCALC_STRLIT_LEN(strlit) (sizeof(strlit) - 1)

// Needs to be a macro instead of an inline struct, since we have to calculate
// the literal string length with sizeof()
#define TCALC_STRLIT_TO_CONST_STRV(strlit) ((const struct tcalc_strv){ .str = (strlit), .len = TCALC_STRLIT_LEN(strlit) })

int tcalc_streq_lb(const char* s1, size_t l1, const char* s2, size_t l2);

// Check if a null terminated string and a length-based string hold equivalent
// information
int tcalc_streq_ntlb(const char* ntstr, const char* lbstr, size_t lbstr_len);

int tcalc_slice_ntstr_eq(const char* source, tcalc_slice slice, const char* ntstr);
/**
 * Taken from FreeBSD
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


/*
 * Taken from FreeBSD
 * Appends src to string dst of size dsize (unlike strncat, dsize is the
 * full size of dst, not space left).  At most dsize-1 characters
 * will be copied.  Always NUL terminates (unless dsize <= strlen(dst)).
 * Returns strlen(src) + MIN(dsize, strlen(initial dst)).
 * If retval >= dsize, truncation occurred.
 *
 * "Note that room for the NUL should be included in [dsize]."
 *  - FreeBSD Library Functions Manual (https://man.freebsd.org/cgi/man.cgi?query=strlcpy&sektion=3)
 */
size_t tcalc_strlcat(char *dst, const char *src, size_t siz);

enum tcalc_err tcalc_strdup(const char *src, char** out);

/**
 * Note that the combined string will be allocated and assigned to *out.
*/
enum tcalc_err tcalc_strcombine(const char *first, const char *second, char** out);

int tcalc_strisint(const char*);
enum tcalc_err tcalc_strtoint(const char*, int*);

int tcalc_strisdouble(const char*);
enum tcalc_err tcalc_strtodouble(const char*, double*);

int tcalc_lpstrisdouble(const char*, size_t);
enum tcalc_err tcalc_lpstrtodouble(const char*, size_t, double*);

enum tcalc_err find_in_strarr(const char**, size_t, const char*, size_t*);
int has_in_strarr(const char**, size_t, const char*);

enum tcalc_err tcalc_strsplit(const char*, char, char***, size_t* return_size);

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
 * @param out the parameter at address out is allocated with the substring upon return value TCALC_ERR_OK
*/
enum tcalc_err tcalc_strsubstr(const char*, size_t, size_t, char**);
#endif
