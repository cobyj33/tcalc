/*  $OpenBSD: strlcpy.c,v 1.16 2019/01/25 00:19:25 millert Exp $    */

/*
 * Copyright (c) 1998, 2015 Todd C. Miller <millert@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "tcalc_string.h"

#include "tcalc_error.h"
#include "tcalc_mem.h"
#include "tcalc_vec.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>

size_t tcalc_strlcpy(char *dst, const char *src, size_t dsize)
{
    const char *osrc = src;
    size_t nleft = dsize;

    /* Copy as many bytes as will fit. */
    if (nleft != 0) {
        while (--nleft != 0) {
            if ((*dst++ = *src++) == '\0')
                break;
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src. */
    if (nleft == 0) {
        if (dsize != 0)
            *dst = '\0';        /* NUL-terminate dst */
        while (*src++)
            ;
    }

    return(src - osrc - 1); /* count does not include NUL */
}

tcalc_err tcalc_strdup(const char *src, char** out) {
    *out = (char*)malloc(sizeof(char) * (strlen(src) + 1));  // Space for length plus nul
    if (*out == NULL) return TCALC_BAD_ALLOC;          // No memory
    strcpy(*out, src);                      // Copy the characters
    return TCALC_OK;                            // Return the new string
}

tcalc_err tcalc_strcombine(const char *first, const char *second, char** out) {
  *out = (char*)calloc((strlen(first) + strlen(second) + 1), sizeof(char));
  if (*out == NULL) return TCALC_BAD_ALLOC;
  strcpy(*out, first);
  strcat(*out, second);
  return TCALC_OK;
}

tcalc_err tcalc_strsubstr(const char* src, size_t start, size_t end, char** out) {
  size_t len = end - start;
  *out = (char*)malloc(sizeof(char) * (len + 1));
  if (*out == NULL) return TCALC_BAD_ALLOC;

  tcalc_strlcpy(*out, src + start, len + 1);
  return TCALC_OK;
}

tcalc_err tcalc_strsplit(const char* str, char split, char*** out, size_t* out_size) {
  tcalc_err err = TCALC_OK;
  TCALC_VEC(char*) strings = TCALC_VEC_INIT;

  size_t start = 0;
  size_t end = 0;
  while (str[end] != '\0') {
    while (str[end] != split && str[end] != '\0')
      end++;
    
    char* substr;
    cleanup_on_err(err, tcalc_strsubstr(str, start, end, &substr));
    TCALC_VEC_PUSH(strings, substr, err);
    if (err) goto cleanup;


    while (str[end] == split && str[end] != '\0')
      end++;
    start = end;
  }

  if (str[start] != '\0') {
    char* substr;
    cleanup_on_err(err, tcalc_strsubstr(str, start, end, &substr));
    TCALC_VEC_PUSH(strings, substr, err);
    if (err) goto cleanup;
  }

  *out = strings.arr;
  *out_size = strings.len;
  return TCALC_OK;

  cleanup:
    TCALC_VEC_FREE_F(strings, free);
    return err;
}

tcalc_err find_in_strarr(const char** list, size_t list_len, const char* search, size_t* out) {
  for (size_t i = 0; i < list_len; i++) {
    if (strcmp(list[i], search) == 0) {
      *out = i;
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

int has_in_strarr(const char** list, size_t list_len, const char* search) {
  size_t dummy;
  return find_in_strarr(list, list_len, search, &dummy) == TCALC_OK;
}

int tcalc_strisint(const char* str) {
  int dummyout;
  return tcalc_strtoint(str, &dummyout) == TCALC_OK;
}

int tcalc_strisdouble(const char* str) {
  double dummyout;
  return tcalc_strtodouble(str, &dummyout) == TCALC_OK;
}

tcalc_err tcalc_strtodouble(const char* str, double* out)
{
  *out = 0.0;

  if (str == NULL)
    return TCALC_INVALID_ARG;
  if (str[0] == '\0')
    return TCALC_INVALID_ARG;


  if (str[0] == '-' || str[0] == '+')  {
    if (str[1] == '.') {
      if (!isdigit(str[2]))
        return TCALC_INVALID_ARG;
    }
    else if (!isdigit(str[1])) {
      return TCALC_INVALID_ARG;
    }
  }

  int foundDecimal = 0;
  double decimalMultiplier = 0.1;
  double sign = str[0] == '-' ? -1.0 : 1.0;

  for (int i = str[0] == '-' || str[0] == '+' ? 1 : 0; str[i] != '\0'; i++) {
    if (str[i] == '.') {
      if (foundDecimal) return TCALC_INVALID_ARG;
      foundDecimal = 1;
    } else if (isdigit(str[i])) {
      if (*out >= (DBL_MAX - 9) / 10) {
        return sign == -1 ? TCALC_UNDERFLOW : TCALC_OVERFLOW;
      }
      
      if (foundDecimal) {
        *out = *out + (str[i] - '0') * decimalMultiplier;
        decimalMultiplier /= 10.0;
      }
      else {
        *out = *out * 10.0 + (str[i] - '0'); 
      }

    } else {
      return TCALC_INVALID_ARG;
    }
  }

  *out *= sign;
  return TCALC_OK;
}

tcalc_err tcalc_strtoint(const char* str, int* out)
{
  *out = 0;
  if (str == NULL)
    return TCALC_INVALID_ARG;
  if (str[0] == '\0')
    return TCALC_INVALID_ARG;

  if (str[0] == '-' || str[0] == '+') { // handles edge case of the str only being "-"
    if (!isdigit(str[1])) {
      return TCALC_INVALID_ARG;
    }
  }

  int sign = str[0] == '-' ? -1 : 1;

  for (int i = str[0] == '-' || str[0] == '+' ? 1 : 0; str[i] != '\0'; i++) {
    if (!isdigit(str[i]))
      return TCALC_INVALID_ARG;
    if (*out >= (INT_MAX - 9) / 10)
      return sign == -1 ? TCALC_UNDERFLOW : TCALC_OVERFLOW;

    *out *= 10;
    *out += str[i] - '0';
  }

  *out *= sign;
  return TCALC_OK;
}

int tcalc_streq(const char* a, const char* b) {
  size_t i = 0;
  for (; a[i] != '\0' && b[i] != '\0'; i++) {
    if (a[i] != b[i]) return 0;
  }

  return a[i] == b[i]; // either a[i] or b[i] will be '\0'. Therefore, if they are equal they have both ended
}