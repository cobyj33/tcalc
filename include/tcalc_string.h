#ifndef TCALC_STRING_H
#define TCALC_STRING_H

#include <stdbool.h>
#include <stddef.h>
#include <tcalc_error.h>

typedef struct tcalc_slice {
  size_t start;
  size_t xend;
} tcalc_slice;


static inline size_t tcalc_slice_len(tcalc_slice slice) {
  return slice.xend - slice.start;
}

#define TCALC_STRLIT_LEN(strlit) (sizeof(strlit) - 1)
#define TCALC_STRLIT_PTR_LEN(strlit) ((strlit)), TCALC_STRLIT_LEN(strlit)

bool tcalc_streq_lblb(const char* s1, size_t l1, const char* s2, size_t l2);

// Check if a null terminated string and a length-based string hold equivalent
// information
bool tcalc_streq_ntlb(const char* ntstr, const char* lbstr, size_t lbstr_len);

bool tcalc_slice_ntstr_eq(const char* source, tcalc_slice slice, const char* ntstr);

size_t tcalc_strlcpy(char *dst, const char *src, size_t dsize);

bool tcalc_lpstrisdouble(const char*, size_t);
enum tcalc_err tcalc_lpstrtodouble(const char*, size_t, double*);

bool tcalc_str_list_has(const char* input, const char** list, size_t count);

bool tcalc_strhaspre(const char* prefix, const char* str);

#endif
