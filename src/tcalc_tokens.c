#include "tcalc_dstring.h"
#include "tcalc_darray.h"
#include "tcalc_error.h"
#include "tcalc_string.h"
#include <stddef.h>
#include <ctype.h>
#include <malloc.h>

int is_valid_tcalc_char(char ch);
tcalc_error_t tcalc_next_math_token(const char* expr, char** out, size_t offset, size_t* new_offset);

const char* ALLOWED_CHARS = "0123456789. ()+-*/^%%";
const char* SINGLE_TOKENS = "()+-*/^%%";

tcalc_error_t tcalc_tokenize(const char* expr, char*** out, size_t* returned_size) {
  tcalc_error_t err;
  *returned_size = 0;
  tcalc_darray* token_buffer = tcalc_darray_alloc(sizeof(char*)); // char**
  if (token_buffer == NULL) return TCALC_BAD_ALLOC;

  #define CLEAN_EARLY_RETURN(pred, tcalc_error) if (pred) { \
                                            tcalc_darray_free_cb(token_buffer, free); \
                                            return tcalc_error; \
                                          }
  #define CLEAN_ERROR(tcalc_error_t_function) CLEAN_EARLY_RETURN((err = tcalc_error_t_function) != TCALC_OK, err)

  tcalc_error_t next_math_token_err = TCALC_OK;
  size_t offset = 0;
  char* current_token;

  while ((next_math_token_err = tcalc_next_math_token(expr, &current_token, offset, &offset)) == TCALC_OK) {
    CLEAN_ERROR(tcalc_darray_push(token_buffer, (void*)&current_token));
  }

  CLEAN_EARLY_RETURN(next_math_token_err != TCALC_STOP_ITER, next_math_token_err)

  CLEAN_ERROR(tcalc_darray_extract(token_buffer, (void**)out));
  *returned_size = tcalc_darray_size(token_buffer);
  tcalc_darray_free(token_buffer); // don't free with callback, as caller will have tokenized strings now
  return TCALC_OK;

  #undef CLEAN_EARLY_RETURN
  #undef CLEAN_ERROR
}

tcalc_error_t tcalc_next_math_token(const char* expr, char** out, size_t start, size_t* new_offset) {
  tcalc_error_t err;
  size_t offset = start;

	while (expr[offset] == ' ') // consume all spaces
		offset++;
  if (expr[offset] == '\0')
    return TCALC_STOP_ITER;

  if (!is_valid_tcalc_char(expr[offset])) 
    return TCALC_INVALID_ARG;

	for (int s = 0; SINGLE_TOKENS[s] != '\0'; s++) { // note that unary plus and minus will not work yet
		if (expr[offset] == SINGLE_TOKENS[s]) {
      if ((err = tcalc_strsubstr(expr, offset, offset + 1, out)) != TCALC_OK) return err;
      *new_offset = offset + 1;
      return TCALC_OK;
		}
	}

	if (isdigit(expr[offset]) || expr[offset] == '.') { // number checking
		if (expr[offset] == '.'  && !isdigit(expr[offset + 1])) { // lone decimal point
      return TCALC_INVALID_ARG;
		}

		int decimalCount = 0;
    size_t numstart = offset;

		while (expr[offset] != '\0' && (isdigit(expr[offset]) || expr[offset] == '.')) {
			if (expr[offset] == '.') {
				if (decimalCount > 0) return TCALC_INVALID_ARG;
				decimalCount++;
			}

      offset++;
		}
		
    if ((err = tcalc_strsubstr(expr, numstart, offset, out))  != TCALC_OK) return err; 
    *new_offset = offset;
    return TCALC_OK;
	}
	
	return TCALC_STOP_ITER;
}


int is_valid_tcalc_char(char ch) {
	for (int i = 0; ALLOWED_CHARS[i] != '\0'; i++)
		if (ALLOWED_CHARS[i] == ch)
			return 1;
	return 0;
}

int tcalc_is_valid_token(const char* token) {
  if (token == NULL) return 0;
  if (token[0] == '\0') return 0; // empty string

  if (token[1] == '\0') { // single character string
    for (int i = 0; SINGLE_TOKENS[i] != '\0'; i++) {
      if (token[0] == SINGLE_TOKENS[i]) return 1;
    }
  }

  if (tcalc_strisdouble(token)) return 1;
  return 0;
}