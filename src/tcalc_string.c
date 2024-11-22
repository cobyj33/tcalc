

#include "tcalc_string.h"

#include "tcalc_error.h"
#include "tcalc_mem.h"
#include "tcalc_vec.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include <assert.h>

/*
 * $OpenBSD: strlcpy.c,v 1.16 2019/01/25 00:19:25 millert Exp $
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

/*	$OpenBSD: strlcat.c,v 1.15 2015/03/02 21:41:08 millert Exp $
 *
 * Copyright (c) 1998, 2015 Todd C. Miller <Todd.Miller@courtesan.com>
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

size_t
tcalc_strlcat(char *dst, const char *src, size_t dsize)
{
	const char *odst = dst;
	const char *osrc = src;
	size_t n = dsize;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end. */
	while (n-- != 0 && *dst != '\0')
		dst++;
	dlen = dst - odst;
	n = dsize - dlen;

	if (n-- == 0)
		return(dlen + strlen(src));
	while (*src != '\0') {
		if (n != 0) {
			*dst++ = *src;
			n--;
		}
		src++;
	}
	*dst = '\0';

	return(dlen + (src - osrc));	/* count does not include NUL */
}


tcalc_err tcalc_strdup(const char *src, char** out) {
    *out = (char*)malloc(sizeof(char) * (strlen(src) + 1));  // Space for length plus nul
    if (*out == NULL) return TCALC_ERR_BAD_ALLOC;          // No memory
    strcpy(*out, src);                      // Copy the characters
    return TCALC_ERR_OK;                            // Return the new string
}

tcalc_err tcalc_strcombine(const char *first, const char *second, char** out) {
  *out = (char*)calloc((strlen(first) + strlen(second) + 1), sizeof(char));
  if (*out == NULL) return TCALC_ERR_BAD_ALLOC;
  strcpy(*out, first);
  strcat(*out, second);
  return TCALC_ERR_OK;
}

tcalc_err tcalc_strsubstr(const char* src, size_t start, size_t end, char** out) {
  size_t len = end - start;
  *out = (char*)malloc(sizeof(char) * (len + 1));
  if (*out == NULL) return TCALC_ERR_BAD_ALLOC;

  tcalc_strlcpy(*out, src + start, len + 1);
  return TCALC_ERR_OK;
}

tcalc_err tcalc_strsplit(const char* str, char split, char*** out, size_t* out_size) {
  tcalc_err err = TCALC_ERR_OK;
  TCALC_VEC(char*) strings = TCALC_VEC_INIT;

  size_t start = 0;
  size_t end = 0;
  while (str[end] != '\0') {
    while (str[end] != split && str[end] != '\0')
      end++;

    char* substr;
    cleanup_on_err(err, tcalc_strsubstr(str, start, end, &substr));
    cleanup_on_macerr(err, TCALC_VEC_PUSH(strings, substr, err));


    while (str[end] == split && str[end] != '\0')
      end++;
    start = end;
  }

  if (str[start] != '\0') {
    char* substr;
    cleanup_on_err(err, tcalc_strsubstr(str, start, end, &substr));
    cleanup_on_macerr(err, TCALC_VEC_PUSH(strings, substr, err));
  }

  *out = strings.arr;
  *out_size = strings.len;
  return TCALC_ERR_OK;

  cleanup:
    TCALC_VEC_FREE_F(strings, free);
    return err;
}

tcalc_err find_in_strarr(const char** list, size_t list_len, const char* search, size_t* out) {
  for (size_t i = 0; i < list_len; i++) {
    if (strcmp(list[i], search) == 0) {
      *out = i;
      return TCALC_ERR_OK;
    }
  }
  return TCALC_ERR_NOT_FOUND;
}

bool has_in_strarr(const char** list, size_t list_len, const char* search) {
  size_t dummy;
  return find_in_strarr(list, list_len, search, &dummy) == TCALC_ERR_OK;
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

bool tcalc_streq(const char* a, const char* b) {
  size_t i = 0;
  for (; a[i] != '\0' && b[i] != '\0'; i++) {
    if (a[i] != b[i]) return false;
  }

  return a[i] == b[i]; // either a[i] or b[i] will be '\0'. Therefore, if they are equal they have both ended
}

bool tcalc_str_list_has(const char* input, const char** list, size_t count) {
  for (size_t i = 0; i < count; i++) {
    if (strcmp(input, list[i]) == 0) return true;
  }
  return false;
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
