#include "tcalc_string.h"
#include "tcalc_darray.h"
#include "tcalc_error.h"
#include <stddef.h>
#include <stdlib.h>

tcalc_error_t tcalc_strsplit(const char* str, char split, char*** out, size_t* return_size) {
  tcalc_error_t err = TCALC_OK;
  tcalc_darray* strings = tcalc_darray_alloc(sizeof(char*)); // char** array
  if (strings == NULL) { return TCALC_BAD_ALLOC; }

  size_t start = 0;
  size_t end = 0;
  while (str[end] != '\0') {
    while (str[end] != split && str[end] != '\0')
      end++;
    
    char* substr;
    if ((err =  tcalc_strsubstr(str, start, end, &substr)) != TCALC_OK) goto cleanup;
    if ((err = tcalc_darray_push(strings, &substr)) != TCALC_OK) goto cleanup;

    while (str[end] == split && str[end] != '\0')
      end++;
    start = end;
  }

  if (str[start] != '\0') {
    char* substr;
    if (( err = tcalc_strsubstr(str, start, end, &substr)) != TCALC_OK) goto cleanup;
    if (( err = tcalc_darray_push(strings, &substr)) != TCALC_OK) goto cleanup;
  }

  if (( err = tcalc_darray_extract(strings, (void**)out)) != TCALC_OK) goto cleanup;
  *return_size = tcalc_darray_size(strings);
  tcalc_darray_free(strings); // do not free individual strings in cb, as they are now passed to the caller
  return TCALC_OK;

  cleanup:
    tcalc_darray_free_cb(strings, free); \
    return err;
}