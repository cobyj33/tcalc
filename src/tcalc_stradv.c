#include "tcalc_string.h"
#include "tcalc_darray.h"
#include "tcalc_error.h"
#include <stddef.h>
#include <stdlib.h>

tcalc_error_t tcalc_strsplit(const char* str, char split, char*** out, size_t* return_size) {
  tcalc_error_t err = TCALC_OK;
  tcalc_darray* strings = tcalc_darray_alloc(sizeof(char*)); // char** array
  if (strings == NULL) { return TCALC_BAD_ALLOC; }

  #define CLEAN_RETURN(predicate, tcalc_error) if (predicate) { tcalc_darray_free_cb(strings, free); \
                                                                return tcalc_error; }
  #define CLEAN_ERROR(tcalc_error_t_func_call) CLEAN_RETURN((err = tcalc_error_t_func_call) != TCALC_OK, err)

  size_t start = 0;
  size_t end = 0;
  while (str[end] != '\0') {
    while (str[end] != split && str[end] != '\0')
      end++;
    
    char* substr;
    CLEAN_ERROR(tcalc_strsubstr(str, start, end, &substr))
    CLEAN_ERROR(tcalc_darray_push(strings, &substr))

    while (str[end] == split && str[end] != '\0')
      end++;
    start = end;
  }

  if (str[start] != '\0') {
    char* substr;
    CLEAN_ERROR(tcalc_strsubstr(str, start, end, &substr))
    CLEAN_ERROR(tcalc_darray_push(strings, &substr))
  }

  *out = (char**)malloc(sizeof(char*) * tcalc_darray_size(strings));
  CLEAN_RETURN(*out == NULL, TCALC_BAD_ALLOC)

  for (size_t i = 0; i < tcalc_darray_size(strings); i++) {
    CLEAN_ERROR(tcalc_darray_at(strings, (void*)(&(*out)[i]), i));
  }

  *return_size = tcalc_darray_size(strings);
  tcalc_darray_free(strings); // do not free individual strings in cb, as they are now passed to the caller
  return TCALC_OK;
  
  #undef CLEAN_ERROR
  #undef CLEAN_RETURN
} 