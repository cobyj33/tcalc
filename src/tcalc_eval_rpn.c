#include "tcalc_eval.h"
#include "tcalc_darray.h"
#include "tcalc_error.h"
#include "tcalc_tokens.h"
#include "tcalc_string.h"
#include "tcalc_mem.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

double tcalc_add(double a, double b) {
  return a + b;
}

double tcalc_subtract(double a, double b) {
  return a - b;
}

double tcalc_multiply(double a, double b) {
  return a * b;
}

double tcalc_divide(double a, double b) {
  return a / b;
}

typedef double (*BinaryOperationFunction)(double, double);

#define SUPPORTED_BINARY_OPERATIONS 5

tcalc_error_t tcalc_eval_rpn(const char* rpn, double* out) {
  char** tokens;
  size_t nb_tokens;
  tcalc_error_t err = tcalc_tokenize_rpn(rpn, &tokens, &nb_tokens);
  if (err) return err;
  
  tcalc_darray* num_stack = tcalc_darray_alloc(sizeof(double));
  if (num_stack == NULL) {
    tcalc_free_arr((void**)tokens, nb_tokens, free);
    return TCALC_BAD_ALLOC;
  }

  const char* operations[SUPPORTED_BINARY_OPERATIONS] = {"+", "-", "*", "/", "^"};
  BinaryOperationFunction operationFunctions[SUPPORTED_BINARY_OPERATIONS] = {tcalc_add, tcalc_subtract, tcalc_multiply, tcalc_divide, pow};

  for (size_t i = 0; i < nb_tokens; i++) {

    size_t matching_operation;
    tcalc_error_t err = find_in_strarr(operations, SUPPORTED_BINARY_OPERATIONS, tokens[i], &matching_operation);

    if (err == TCALC_OK) {

      if (tcalc_darray_size(num_stack) < 2) {
        tcalc_free_arr((void**)tokens, nb_tokens, free);
        tcalc_darray_free(num_stack);
        return TCALC_INVALID_ARG;
      }

      double first, second;
      tcalc_darray_pop(num_stack, &second);
      tcalc_darray_pop(num_stack, &first);
      double res = operationFunctions[matching_operation](first, second);
      tcalc_darray_push(num_stack, &res);

    } else if (tcalc_strisdouble(tokens[i])) {
      double out;
      tcalc_strtodouble(tokens[i], &out);
      tcalc_darray_push(num_stack, &out);
    } else {
      tcalc_free_arr((void**)tokens, nb_tokens, free);
      tcalc_darray_free(num_stack);
      return TCALC_INVALID_ARG;
    }

  }

  err = TCALC_OK;
  if (tcalc_darray_size(num_stack) == 1) {
    tcalc_darray_pop(num_stack, (void*)out);
  } else {
    err = TCALC_INVALID_ARG;
  }

  tcalc_free_arr((void**)tokens, nb_tokens, free);
  tcalc_darray_free(num_stack);
  return err;
}

#undef SUPPORTED_BINARY_OPERATIONS
