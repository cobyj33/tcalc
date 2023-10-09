#include "tcalc_eval.h"
#include "tcalc_darray.h"
#include "tcalc_error.h"
#include "tcalc_tokens.h"
#include "tcalc_string.h"
#include "tcalc_mem.h"
#include "tcalc_func.h"

#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <float.h>



// #define SUPPORTED_BINARY_OPERATIONS 5

// tcalc_error_t tcalc_eval_rpn(const char* rpn, double* out) {
//   char** tokens;
//   size_t nb_tokens;
//   tcalc_error_t err = tcalc_tokenize_rpn(rpn, &tokens, &nb_tokens);
//   if (err) return err;
  
//   tcalc_darray* num_stack = tcalc_darray_alloc(sizeof(double));
//   if (num_stack == NULL) {
//     tcalc_free_arr((void**)tokens, nb_tokens, free);
//     return TCALC_BAD_ALLOC;
//   }

//   const char* operations[SUPPORTED_BINARY_OPERATIONS] = {"+", "-", "*", "/", "^"};
//   tcalc_binary_op_func operationFunctions[SUPPORTED_BINARY_OPERATIONS] = {tcalc_add, tcalc_subtract, tcalc_multiply, tcalc_divide, tcalc_pow};

//   for (size_t i = 0; i < nb_tokens; i++) {

//     size_t matching_operation;
//     tcalc_error_t could_find_binary_op = find_in_strarr(operations, SUPPORTED_BINARY_OPERATIONS, tokens[i], &matching_operation);

//     if (could_find_binary_op == TCALC_OK) {

//       if (tcalc_darray_size(num_stack) < 2) {
//         tcalc_free_arr((void**)tokens, nb_tokens, free);
//         tcalc_darray_free(num_stack);
//         return TCALC_INVALID_ARG;
//       }

//       double first, second, res;
//       tcalc_darray_pop(num_stack, &second);
//       tcalc_darray_pop(num_stack, &first);
//       tcalc_error_t operator_err = operationFunctions[matching_operation](first, second, &res);

//       if (operator_err) {
//         tcalc_free_arr((void**)tokens, nb_tokens, free);
//         tcalc_darray_free(num_stack);
//         return operator_err;
//       }

//       tcalc_darray_push(num_stack, &res);

//     } else if (tcalc_strisdouble(tokens[i])) {
//       double out;
//       tcalc_strtodouble(tokens[i], &out);
//       tcalc_darray_push(num_stack, &out);
//     } else {
//       tcalc_free_arr((void**)tokens, nb_tokens, free);
//       tcalc_darray_free(num_stack);
//       return TCALC_INVALID_ARG;
//     }

//   }

//   err = TCALC_OK;
//   if (tcalc_darray_size(num_stack) == 1) {
//     tcalc_darray_pop(num_stack, (void*)out);
//   } else {
//     err = TCALC_INVALID_ARG;
//   }

//   tcalc_free_arr((void**)tokens, nb_tokens, free);
//   tcalc_darray_free(num_stack);
//   return err;
// }

// #undef SUPPORTED_BINARY_OPERATIONS