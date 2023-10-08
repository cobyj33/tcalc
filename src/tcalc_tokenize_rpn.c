#include "tcalc_tokens.h"
#include "tcalc_string.h"
#include "tcalc_mem.h"
#include <stdlib.h>

tcalc_error_t tcalc_tokenize_rpn(const char* expr, char*** out, size_t* returned_size) {
  *returned_size = 0;
  tcalc_error_t err = tcalc_strsplit(expr, ' ', out, returned_size); // very simple :)
  if (err) return err;

  for (int i = 0; i < *returned_size; i++) {
    if (!tcalc_is_valid_token((*out)[i])) {
      tcalc_free_arr((void**)(*out), *returned_size, free);
      *returned_size = 0;
      return TCALC_INVALID_ARG;
    }
  }
  
  return TCALC_OK;
}