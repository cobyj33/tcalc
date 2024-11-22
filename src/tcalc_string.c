

#include "tcalc_string.h"

#include "tcalc_error.h"
#include "tcalc_mem.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include <assert.h>

// TODO: Maybe we shouldn't try to emulate strlcpy? It's quite bad
size_t tcalc_strlcpy(char *dst, const char *src, size_t dsize)
{
  strncpy(dst, src, dsize);
  if (dsize > 0)
    dst[dsize - 1] = '\0';
  return strlen(src);
}

bool tcalc_lpstrisdouble(const char* str, size_t len) {
  double dummyout;
  return tcalc_lpstrtodouble(str, len, &dummyout) == TCALC_ERR_OK;
}

tcalc_err tcalc_lpstrtodouble(const char* str, size_t len, double* out) {
  assert(str != NULL);
  assert(out != NULL);
  *out = 0.0;

  if (len == 0)
    return TCALC_ERR_INVALID_ARG;

  if (str[0] == '-' || str[0] == '+')  {
    if (str[1] == '.') {
      if (!isdigit(str[2]))
        return TCALC_ERR_INVALID_ARG;
    }
    else if (!isdigit(str[1])) {
      return TCALC_ERR_INVALID_ARG;
    }
  }

  int foundDecimal = 0;
  double decimalMultiplier = 0.1;
  double sign = str[0] == '-' ? -1.0 : 1.0;

  for (size_t i = (str[0] == '-' || str[0] == '+'); i < len; i++) {
    if (str[i] == '.') {
      if (foundDecimal) return TCALC_ERR_INVALID_ARG;
      foundDecimal = 1;
    } else if (isdigit(str[i])) {
      if (*out >= (DBL_MAX - 9) / 10) {
        return sign == -1 ? TCALC_ERR_UNDERFLOW : TCALC_ERR_OVERFLOW;
      }

      if (foundDecimal) {
        *out = *out + (str[i] - '0') * decimalMultiplier;
        decimalMultiplier /= 10.0;
      }
      else {
        *out = *out * 10.0 + (str[i] - '0');
      }

    } else {
      return TCALC_ERR_INVALID_ARG;
    }
  }

  *out *= sign;
  return TCALC_ERR_OK;
}

bool tcalc_str_list_has(const char* input, const char** list, size_t count) {
  size_t i = 0;
  while (i < count && (strcmp(input, list[i]) != 0)) i++;
  return i < count;
}

bool tcalc_strhaspre(const char* prefix, const char* str) {
  size_t i = 0;
  while (str[i] != '\0' && prefix[i] != '\0' && prefix[i] == str[i])
    i++;
  return prefix[i] == '\0';
}

bool tcalc_streq_lb(const char* s1, size_t l1, const char* s2, size_t l2) {
  if (l1 != l2) return 0;
  const char* const e1 = s1 + l1;
  while (s1 != e1 && *s1 == *s2) { s1++; s2++; }
  return s1 == e1;
}

bool tcalc_streq_ntlb(const char* ntstr, const char* lbstr, size_t lbstr_len) {
  const char* const lbstr_end = lbstr + lbstr_len;
  while (*ntstr && lbstr != lbstr_end && *ntstr == *lbstr) { ntstr++; lbstr++; }
  return *ntstr == '\0' && (lbstr == lbstr_end);
}

bool tcalc_slice_ntstr_eq(const char* source, tcalc_slice slice, const char* ntstr) {
  assert(source != NULL);
  assert(slice.xend >= slice.start);
  assert(ntstr != NULL);
  return tcalc_streq_ntlb(ntstr, source + slice.start, tcalc_slice_len(slice));
}
