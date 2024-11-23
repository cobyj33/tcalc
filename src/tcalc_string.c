
#include "tcalc.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include <assert.h>

// TODO: Maybe we shouldn't try to emulate strlcpy? It's quite bad
int32_t tcalc_strcpy_lblb(char *dest, int32_t destCapacity, const char *src, int32_t srcLen)
{
  const int32_t copied = TCALC_MIN_UNSAFE(destCapacity, srcLen);
  memcpy(dest, src, (size_t)copied);
  return copied;
}

int32_t tcalc_strcpy_lblb_ntdst(char* dst, int32_t dstCapacity, const char* src, int32_t srcLen)
{
  int32_t copied = tcalc_strcpy_lblb(dst, dstCapacity, src, srcLen);
  if (copied < dstCapacity)
    dst[copied + 1] = '\0';
  else if (copied == dstCapacity && copied > 0)
    dst[copied--] = '\0';
  return copied;
}

bool tcalc_lpstrisdouble(const char* str, size_t len) {
  double dummyout = 0.0;
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
      if (!tcalc_is_digit(str[2]))
        return TCALC_ERR_INVALID_ARG;
    }
    else if (!tcalc_is_digit(str[1])) {
      return TCALC_ERR_INVALID_ARG;
    }
  }

  bool foundDecimal = false;
  double decimalMultiplier = 0.1;
  double sign = str[0] == '-' ? -1.0 : 1.0;

  for (size_t i = (str[0] == '-' || str[0] == '+'); i < len; i++) {
    if (str[i] == '.') {
      if (foundDecimal) return TCALC_ERR_INVALID_ARG;
      foundDecimal = true;
    } else if (tcalc_is_digit(str[i])) {
      if (!foundDecimal && *out >= (DBL_MAX - 9) / 10) {
        return TCALC_ERR_OVERFLOW;
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

bool tcalc_strhaspre(const char* prefix, int32_t prefixLen, const char* str, int32_t strLen) {
  int32_t i = 0;
  if (prefixLen > strLen) return false;
  while (i < prefixLen && i < strLen && prefix[i] == str[i])
    i++;
  return i == prefixLen;
}

bool tcalc_streq_lblb(const char* s1, size_t l1, const char* s2, size_t l2) {
  if (l1 != l2) return false;
  const char* const e1 = s1 + l1;
  while (s1 != e1 && *s1 == *s2) { s1++; s2++; }
  return s1 == e1;
}

bool tcalc_streq_ntlb(const char* ntstr, const char* lbstr, int32_t lbstr_len) {
  int32_t i = 0;
  while (ntstr[i] != '\0' && i < lbstr_len && ntstr[i] == lbstr[i]) i++;
  return ntstr[i] == '\0' && (i == lbstr_len);
}
