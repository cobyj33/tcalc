#include "tcalc_eval.h"
#include "tcalc_dstring.h"
#include "tcalc_darray.h"
#include "tcalc_tokens.h"
#include "tcalc_mem.h"
#include "tcalc_string.h"

#include <stdlib.h>
#include <string.h>

typedef enum {
  TCALC_RIGHT_ASSOCIATIVE,
  TCALC_LEFT_ASSOCIATIVE,
} tcalc_associativity_t;

typedef struct {
  const char* token;
  int priority;
  tcalc_associativity_t associativity;
} operator_data;

tcalc_error_t tcalc_find_matching_operator(const operator_data* operations, size_t nb_operations, char* token, operator_data* out) {
  for (int i = 0; i < nb_operations; i++) {
    if (strcmp(token, operations[i].token) == 0) {
      *out = operations[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

tcalc_error_t tcalc_infix_to_rpn(const char* infix, char** out) {
  tcalc_error_t err;
  #define OPERATOR_DEFINITION_COUNT 5
  const operator_data OPERATOR_DEFINITIONS[OPERATOR_DEFINITION_COUNT] = {
    {"+", 1, TCALC_LEFT_ASSOCIATIVE},
    {"-", 1, TCALC_LEFT_ASSOCIATIVE},
    {"/", 2, TCALC_LEFT_ASSOCIATIVE},
    {"*", 2, TCALC_LEFT_ASSOCIATIVE},
    {"^", 3, TCALC_RIGHT_ASSOCIATIVE},
  };

  tcalc_darray* operator_stack = tcalc_darray_alloc(sizeof(char*)); // char** stack
  if (operator_stack == NULL) {
    return TCALC_BAD_ALLOC;
  }

  tcalc_darray* output_buffer = tcalc_darray_alloc(sizeof(char*)); // char** array, will be joined
  if (output_buffer == NULL) {
    tcalc_darray_free(operator_stack);
    return TCALC_BAD_ALLOC;
  }

  tcalc_dstring* joined_string = tcalc_dstring_alloc();
  if (joined_string == NULL) {
    tcalc_darray_free(operator_stack);
    tcalc_darray_free(output_buffer);
    return TCALC_BAD_ALLOC;
  }

  char** tokens;
  size_t nb_tokens;
  err = tcalc_tokenize_infix(infix, &tokens, &nb_tokens);
  if (err) {
    tcalc_darray_free(operator_stack);
    tcalc_darray_free(output_buffer);
    tcalc_dstring_free(joined_string);
    return err;
  }

  #define CLEAN_RETURN(predicate, tcalc_error) if (predicate) { tcalc_darray_free(operator_stack); \
                                  tcalc_darray_free(output_buffer); \
                                  tcalc_dstring_free(joined_string); \
                                  tcalc_free_arr((void**)tokens, nb_tokens, free); \
                                  return tcalc_error; }
  #define CLEAN_ERROR(tcalc_error_t_func_call) CLEAN_RETURN((err = tcalc_error_t_func_call) != TCALC_OK, err)
  // macros allow us to easily call errneous code without so many if blocks

  for (size_t i = 0; i < nb_tokens; i++) {
    if (tcalc_strisdouble(tokens[i])) {
      CLEAN_ERROR(tcalc_darray_push(output_buffer, (void*)&(tokens[i]) ) );
    }
    else if (strcmp(tokens[i], "(") == 0) {
      CLEAN_ERROR(tcalc_darray_push(operator_stack, &tokens[i]))
    }
    else if (strcmp(tokens[i], ")") == 0) { // pop everything from operator stack onto the
      char* operator_token;
      CLEAN_ERROR(tcalc_darray_pop(operator_stack, &operator_token))
      while (strcmp(operator_token, "(") != 0) { // keep popping onto output until the opening parentheses is found
        CLEAN_RETURN(tcalc_darray_size(operator_stack) == 0, TCALC_BAD_ALLOC);

        CLEAN_ERROR(tcalc_darray_push(output_buffer, &operator_token))
        CLEAN_ERROR(tcalc_darray_pop(operator_stack, &operator_token))
      }
    } else { // must be an operator
      operator_data current_opt, stack_top_opt;
      CLEAN_ERROR(tcalc_find_matching_operator(OPERATOR_DEFINITIONS, OPERATOR_DEFINITION_COUNT, tokens[i], &current_opt));

      if (tcalc_darray_size(operator_stack) > 0) {
        char* stack_top;
        CLEAN_ERROR(tcalc_darray_peek(operator_stack, &stack_top))

        while (tcalc_find_matching_operator(OPERATOR_DEFINITIONS, OPERATOR_DEFINITION_COUNT, stack_top, &stack_top_opt) == TCALC_OK) {
          if (stack_top_opt.priority > current_opt.priority || stack_top_opt.priority == current_opt.priority && current_opt.associativity == TCALC_LEFT_ASSOCIATIVE) {
            CLEAN_ERROR(tcalc_darray_push(output_buffer, &stack_top))
          } else { break; }
          
          CLEAN_ERROR(tcalc_darray_pop(operator_stack, NULL))
          if (tcalc_darray_size(operator_stack) == 0) break;

          CLEAN_ERROR(tcalc_darray_peek(operator_stack, &stack_top))
        }
      }

      CLEAN_ERROR(tcalc_darray_push(operator_stack, &tokens[i]))
    }
  }

  while (tcalc_darray_size(operator_stack) > 0) { // pop all remaining data onto output
    char* token;
    CLEAN_ERROR(tcalc_darray_pop(operator_stack, &token))
    CLEAN_ERROR(tcalc_darray_push(output_buffer, &token))
  }

  CLEAN_RETURN(joined_string == NULL, TCALC_BAD_ALLOC)

  for (size_t i = 0; i < tcalc_darray_size(output_buffer); i++) {
    char* token;
    CLEAN_ERROR(tcalc_darray_at(output_buffer, &token, i))
    CLEAN_ERROR(tcalc_dstring_append_cstr(joined_string, token))

    if (i != tcalc_darray_size(output_buffer) - 1) {
      CLEAN_ERROR(tcalc_dstring_append_cstr(joined_string, " "))
    }
  }

  err = tcalc_dstring_cstrdup(joined_string, out);
  CLEAN_RETURN(1, err)
  #undef CLEAN_RETURN
  #undef CLEAN_ERROR
}
