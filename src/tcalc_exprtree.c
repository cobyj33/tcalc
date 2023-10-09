#include "tcalc_exprtree.h"

#include "tcalc_string.h"
#include "tcalc_func.h"
#include "tcalc_darray.h"

tcalc_error_t tcalc_eval_exprtree(tcalc_exprtree_t* expr, double* out) {
  switch (expr->token.type) {
    case TCALC_NUMBER: {
      tcalc_error_t err = tcalc_strtodouble(expr->token.value, out);
      if (err) return err;
      return TCALC_OK;
    }
    case TCALC_UNARY_OPERATOR: {
      if (expr->nb_children != 1) return TCALC_INVALID_ARG;

      double operand;
      tcalc_error_t err = tcalc_eval_exprtree(expr->children[0], &operand);
      if (err) return err;

      if (strcmp(expr->token.value, "+") == 0) {
        *out = operand;
        return TCALC_OK;
      } else if (strcmp(expr->token.value, "-") == 0) {
        *out = -operand;
        return TCALC_OK;
      } else {
        return TCALC_INVALID_ARG;
      } 
    }
    case TCALC_BINARY_OPERATOR: {
      if (expr->nb_children != 2) return TCALC_INVALID_ARG;
      double operand1;
      double operand2;
      tcalc_error_t err = tcalc_eval_exprtree(expr->children[0], &operand1);
      if (err) return err;
      err = tcalc_eval_exprtree(expr->children[1], &operand2);
      if (err) return err;
      
      if (strcmp(expr->token.value, "+") == 0) {
        return tcalc_add(operand1, operand2, out);
      } else if (strcmp(expr->token.value, "-") == 0) {
        return tcalc_subtract(operand1, operand2, out);
      } else if (strcmp(expr->token.value, "*") == 0) {
        return tcalc_multiply(operand1, operand2, out);
      } else if (strcmp(expr->token.value, "/") == 0) {
        return tcalc_divide(operand1, operand2, out);
      } else if (strcmp(expr->token.value, "^") == 0) {
        return tcalc_pow(operand1, operand2, out);
      } else {
        return TCALC_INVALID_ARG;
      } 
    }
    default: {
      return TCALC_INVALID_ARG;
    }
  }
}

#define SUPPORTED_BINARY_OPERATIONS 5

tcalc_error_t tcalc_rpn_tokens_to_exprtree(tcalc_token_t* tokens, size_t nb_tokens, tcalc_exprtree_t** out) {
  tcalc_darray* num_stack = tcalc_darray_alloc(sizeof(double));
  if (num_stack == NULL) return TCALC_BAD_ALLOC;

  const char* operations[SUPPORTED_BINARY_OPERATIONS] = {"+", "-", "*", "/", "^"};
  tcalc_binary_op_func operationFunctions[SUPPORTED_BINARY_OPERATIONS] = {tcalc_add, tcalc_subtract, tcalc_multiply, tcalc_divide, tcalc_pow};

  for (size_t i = 0; i < nb_tokens; i++) {

    size_t matching_operation;
    tcalc_error_t could_find_binary_op = find_in_strarr(operations, SUPPORTED_BINARY_OPERATIONS, tokens[i], &matching_operation);

    if (could_find_binary_op == TCALC_OK) {

      if (tcalc_darray_size(num_stack) < 2) {
        tcalc_free_arr((void**)tokens, nb_tokens, free);
        tcalc_darray_free(num_stack);
        return TCALC_INVALID_ARG;
      }

      double first, second, res;
      tcalc_darray_pop(num_stack, &second);
      tcalc_darray_pop(num_stack, &first);
      tcalc_error_t operator_err = operationFunctions[matching_operation](first, second, &res);

      if (operator_err) {
        tcalc_free_arr((void**)tokens, nb_tokens, free);
        tcalc_darray_free(num_stack);
        return operator_err;
      }

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

typedef struct {
  const char* token;
  int priority;
  tcalc_associativity_t associativity;
} tcalc_binary_op_data_t;

tcalc_error_t tcalc_index_of_binary_op_data(const tcalc_binary_op_data_t* operations, size_t nb_operations, char* token, tcalc_binary_op_data_t* out) {
  for (int i = 0; i < nb_operations; i++) {
    if (strcmp(token, operations[i].token) == 0) {
      *out = operations[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}



tcalc_error_t tcalc_infix_tokens_to_rpn_tokens(tcalc_token_t* infix_tokens, size_t nb_infix_tokens, tcalc_token_t** out, size_t* out_size) {
  tcalc_error_t err;
  #define OPERATOR_DEFINITION_COUNT 5
  const tcalc_binary_op_data_t OPERATOR_DEFINITIONS[OPERATOR_DEFINITION_COUNT] = {
    {"+", 1, TCALC_LEFT_ASSOCIATIVE},
    {"-", 1, TCALC_LEFT_ASSOCIATIVE},
    {"/", 2, TCALC_LEFT_ASSOCIATIVE},
    {"*", 2, TCALC_LEFT_ASSOCIATIVE},
    {"^", 3, TCALC_RIGHT_ASSOCIATIVE},
  };

  tcalc_darray* operator_stack = tcalc_darray_alloc(sizeof(tcalc_token_t)); // tcalc_token_t** stack
  if (operator_stack == NULL) {
    return TCALC_BAD_ALLOC;
  }

  tcalc_darray* output_buffer = tcalc_darray_alloc(sizeof(tcalc_token_t)); // tcalc_token_t* array, will be joined
  if (output_buffer == NULL) {
    tcalc_darray_free(operator_stack);
    return TCALC_BAD_ALLOC;
  }

  #define CLEAN_RETURN(predicate, tcalc_error) if (predicate) { \
                                  tcalc_darray_free(operator_stack); \
                                  tcalc_darray_free(output_buffer); \
                                  return tcalc_error; }

  #define CLEAN_ERROR(tcalc_error_t_func_call) CLEAN_RETURN((err = tcalc_error_t_func_call) != TCALC_OK, err)
  // macros allow us to easily call errneous code without so many if blocks

  for (size_t i = 0; i < nb_tokens; i++) {
    switch (tokens[i].type)

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
      tcalc_binary_op_data_t current_opt, stack_top_opt;
      CLEAN_ERROR(tcalc_index_of_binary_op_data(OPERATOR_DEFINITIONS, OPERATOR_DEFINITION_COUNT, tokens[i], &current_opt));

      if (tcalc_darray_size(operator_stack) > 0) {
        char* stack_top;
        CLEAN_ERROR(tcalc_darray_peek(operator_stack, &stack_top))

        while (tcalc_index_of_binary_op_data(OPERATOR_DEFINITIONS, OPERATOR_DEFINITION_COUNT, stack_top, &stack_top_opt) == TCALC_OK) {
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

tcalc_error_t tcalc_create_exprtree_infix(const char* infix, tcalc_exprtree_t** out) {
  tcalc_token_t* infix_tokens;
  size_t nb_infix_tokens;
  tcalc_error_t err = tcalc_tokenize_infix(infix, &infix_tokens, &nb_infix_tokens);
  if (err) return err;

  tcalc_token_t* rpn_tokens;
  size_t nb_rpn_tokens;
  err = tcalc_infix_tokens_to_rpn_tokens(infix_tokens, nb_infix_tokens, &rpn_tokens, nb_rpn_tokens);
  if (err) {

  }

  err = tcalc_rpn_tokens_to_exprtree(rpn_tokens, nb_rpn_tokens, out);
  if (err) {

  }

  return TCALC_OK;
}

tcalc_error_t tcalc_create_exprtree_rpn(const char* rpn, tcalc_exprtree_t** out) {
  tcalc_token_t* tokens;
  size_t nb_tokens;
  tcalc_error_t err = tcalc_tokenize_rpn(rpn, &tokens, &nb_tokens);
  if (err) return err;
  err = tcalc_rpn_tokens_to_exprtree(tokens, out);

  if (err) {
    for (int i = 0; i < nb_tokens; i++)
      free(tokens[i].value);
    free(tokens);
  }

  return err;
}

