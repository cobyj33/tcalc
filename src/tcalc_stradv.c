#include "tcalc_string.h"
#include "tcalc_darray.h"
#include "tcalc_error.h"

#include "tcalc_mem.h"
#include <stddef.h>
#include <stdlib.h>

tcalc_error_t tcalc_strsplit(const char* str, char split, char*** out, size_t* out_size) {
  tcalc_error_t err = TCALC_OK;
  char** strings = NULL;
  size_t strings_size = 0;
  size_t strings_capacity = 0;

  size_t start = 0;
  size_t end = 0;
  while (str[end] != '\0') {
    while (str[end] != split && str[end] != '\0')
      end++;
    
    char* substr;
    if ((err =  tcalc_strsubstr(str, start, end, &substr)) != TCALC_OK) goto cleanup;
    if ((err = tcalc_alloc_grow(&strings, sizeof(char*), strings_size, &strings_capacity)) != TCALC_OK) goto cleanup;
    strings[strings_size++] = substr;

    while (str[end] == split && str[end] != '\0')
      end++;
    start = end;
  }

  if (str[start] != '\0') {
    char* substr;
    if (( err = tcalc_strsubstr(str, start, end, &substr)) != TCALC_OK) goto cleanup;
    if ((err = tcalc_alloc_grow(&strings, sizeof(char*), strings_size, &strings_capacity)) != TCALC_OK) goto cleanup;
    strings[strings_size++] = substr;
  }

  *out = strings;
  *out_size = strings_size;
  return TCALC_OK;

  cleanup:
    tcalc_free_arr(strings, strings_size, free);
    return err;
}