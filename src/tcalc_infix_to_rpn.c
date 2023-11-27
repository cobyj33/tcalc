#include "tcalc_context.h"
#include "tcalc_tokens.h"
#include "tcalc_error.h"
#include "tcalc_mem.h"

#include <stdlib.h>
#include <string.h>

tcalc_error_t tcalc_context_get_token_op_data(const tcalc_context_t* context, tcalc_token_t* token, tcalc_op_data_t* out) {
  tcalc_error_t err = TCALC_OK;

  switch (token->type) {
    case TCALC_UNARY_OPERATOR: {
      tcalc_unary_op_def_t* unary_op_def;
      if ((err = tcalc_context_get_unary_op(context, token->value, &unary_op_def)) != TCALC_OK) return err;
      *out = tcalc_unary_op_get_data(unary_op_def);
      return TCALC_OK;
    }
    case TCALC_BINARY_OPERATOR: {
      tcalc_binary_op_def_t* binary_op_def;
      if ((err = tcalc_context_get_binary_op(context, token->value, &binary_op_def)) != TCALC_OK) return err;
      *out = tcalc_binary_op_get_data(binary_op_def);
      return TCALC_OK;
    }
    default: return TCALC_INVALID_ARG;
  }

  return TCALC_INVALID_ARG;
}

/**
 * Implementation of the shunting yard algorithm to reorder infix-formatted
 * tokens into an rpn-style. Used for further processing of tokens and to help
 * the ease of creating an AST tree from tokens later on.
 * 
 * https://en.wikipedia.org/wiki/Shunting_yard_algorithm
 * 
 * Remember that the number of infix tokens and the number of rpn tokens are not
 * necessarily the same, as rpn doesn't need grouping tokens.
 * 
 * Upon returning TCALC_OK, *out is an allocated array of size *out_size. The caller
 * is responsible for freeing these tokens.
*/
tcalc_error_t tcalc_infix_tokens_to_rpn_tokens(tcalc_token_t** infix_tokens, size_t nb_infix_tokens, const tcalc_context_t* context, tcalc_token_t*** out, size_t* out_size) {
  tcalc_error_t err;

  tcalc_token_t** operator_stack = (tcalc_token_t**)malloc(sizeof(tcalc_token_t*) * nb_infix_tokens);
  if (operator_stack == NULL) return TCALC_BAD_ALLOC;
  size_t operator_stack_size = 0;

  tcalc_token_t** rpn_tokens = (tcalc_token_t**)malloc(sizeof(tcalc_token_t*) * nb_infix_tokens); // tcalc_token_t* array, will be joined
  size_t rpn_tokens_size = 0;
  if (rpn_tokens == NULL) {
    free(operator_stack);
    return TCALC_BAD_ALLOC;
  }

  for (size_t i = 0; i < nb_infix_tokens; i++) {
    switch (infix_tokens[i]->type) {
      case TCALC_NUMBER: {
        if ((err = tcalc_token_clone(infix_tokens[i], &rpn_tokens[rpn_tokens_size])) != TCALC_OK) goto cleanup;
        rpn_tokens_size++;
        break;
      }
      case TCALC_GROUP_START: { // "(" or "["
        operator_stack[operator_stack_size++] = infix_tokens[i];
        break;
      }
      case TCALC_GROUP_END: { // ")" or "]"
        if (operator_stack_size == 0) {
          err = TCALC_INVALID_OP;
          goto cleanup;
        }

        const char* opener;
        if (strcmp(infix_tokens[i]->value, ")") == 0) {
          opener = "(";
        } else if (strcmp(infix_tokens[i]->value, "]") == 0) {
          opener = "[";
        } else {
          err = TCALC_INVALID_OP;
          goto cleanup;
        }

        while (strcmp(operator_stack[operator_stack_size - 1]->value, opener) != 0) { // keep popping onto output until the opening parenthesis is found
          if ((err = tcalc_token_clone(operator_stack[operator_stack_size - 1], &rpn_tokens[rpn_tokens_size])) != TCALC_OK) goto cleanup;
          rpn_tokens_size++;
          operator_stack_size--;
          
          if (operator_stack_size == 0) {
            err = TCALC_INVALID_OP;
            goto cleanup;
          }
        }
        operator_stack_size--; // pop off opening grouping symbol

        if (operator_stack_size >= 1) {
          if (tcalc_context_has_func(context, operator_stack[operator_stack_size - 1]->value)) {
              if ((err = tcalc_token_clone(operator_stack[operator_stack_size - 1], &rpn_tokens[rpn_tokens_size])) != TCALC_OK) goto cleanup;
              rpn_tokens_size++;
              operator_stack_size--;
          }
        }

        break;
      }
      case TCALC_UNARY_OPERATOR:
      case TCALC_BINARY_OPERATOR: {
        tcalc_op_data_t current_optdata, stack_optdata;
        if ((err = tcalc_context_get_token_op_data(context, infix_tokens[i], &current_optdata))) goto cleanup;

        while (operator_stack_size > 0) {
          tcalc_token_t* stack_top = operator_stack[operator_stack_size - 1];
          if (stack_top->type == TCALC_GROUP_START) break;

          if ((err = tcalc_context_get_token_op_data(context, stack_top, &stack_optdata))) goto cleanup;
          if (current_optdata.precedence > stack_optdata.precedence) break;
          if (current_optdata.precedence == stack_optdata.precedence && current_optdata.associativity == TCALC_RIGHT_ASSOCIATIVE) break;

          if ((err = tcalc_token_clone(operator_stack[operator_stack_size - 1], &rpn_tokens[rpn_tokens_size])) != TCALC_OK) goto cleanup;
          rpn_tokens_size++;
          operator_stack_size--;
        }

        operator_stack[operator_stack_size++] = infix_tokens[i];
        break;
      }
      case TCALC_IDENTIFIER: {  
        if (tcalc_context_has_func(context, infix_tokens[i]->value)) {
          operator_stack[operator_stack_size++] = infix_tokens[i];
        } else if (tcalc_context_has_variable(context, infix_tokens[i]->value)) {
          if ((err = tcalc_token_clone(infix_tokens[i], &rpn_tokens[rpn_tokens_size])) != TCALC_OK) goto cleanup;
          rpn_tokens_size++;
        } else {
          err = TCALC_UNKNOWN_IDENTIFIER;
          goto cleanup;
        }

        break;
      }
      case TCALC_PARAM_SEPARATOR: {
        if (operator_stack_size == 0) {
          err = TCALC_INVALID_OP;
          goto cleanup;
        }

        while (operator_stack[operator_stack_size - 1]->type != TCALC_GROUP_START) { // keep popping onto output until the opening grouping symbol is found
          if ((err = tcalc_token_clone(operator_stack[operator_stack_size - 1], &rpn_tokens[rpn_tokens_size])) != TCALC_OK) goto cleanup;
          rpn_tokens_size++;
          operator_stack_size--;
        }

        break;
      }
      default: {
        err = TCALC_UNIMPLEMENTED;
        goto cleanup;
      }
    }
  }

  while (operator_stack_size > 0) {
    if ((err = tcalc_token_clone(operator_stack[operator_stack_size - 1], &rpn_tokens[rpn_tokens_size])) != TCALC_OK) goto cleanup;
    rpn_tokens_size++;
    operator_stack_size--;
  }

  free(operator_stack);
  *out = rpn_tokens;
  *out_size = rpn_tokens_size;
  return TCALC_OK;

  cleanup:
    free(operator_stack);
    tcalc_free_arr((void**)rpn_tokens, rpn_tokens_size, tcalc_token_freev);
    *out_size = 0;
    return err;
}

