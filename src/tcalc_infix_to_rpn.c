#include "tcalc_exprtree.h"
#include "tcalc_context.h"
#include "tcalc_tokens.h"
#include "tcalc_error.h"
#include "tcalc_mem.h"

#include <stdlib.h>
#include <string.h>


typedef struct {
  tcalc_token_t token;
  int priority;
  tcalc_associativity_t associativity;
} tcalc_op_precedence_t;

tcalc_error_t tcalc_get_prec_data(const tcalc_op_precedence_t* operations, size_t nb_operations, tcalc_token_t* token, tcalc_op_precedence_t* out) {
  for (size_t i = 0; i < nb_operations; i++) {
    if (token->type == operations[i].token.type && strcmp(token->value, operations[i].token.value) == 0) {
      *out = operations[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
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
  #define OP_PRECEDENCE_DEF_COUNT 8

  const tcalc_op_precedence_t OP_PRECEDENCE_DEFS[OP_PRECEDENCE_DEF_COUNT] = {
    {{TCALC_BINARY_OPERATOR, "+"}, 1, TCALC_LEFT_ASSOCIATIVE},
    {{TCALC_BINARY_OPERATOR, "-"}, 1, TCALC_LEFT_ASSOCIATIVE},
    {{TCALC_BINARY_OPERATOR, "*"}, 2, TCALC_LEFT_ASSOCIATIVE},
    {{TCALC_BINARY_OPERATOR, "/"}, 2, TCALC_LEFT_ASSOCIATIVE},
    {{TCALC_BINARY_OPERATOR, "%"}, 2, TCALC_LEFT_ASSOCIATIVE},
    {{TCALC_BINARY_OPERATOR, "^"}, 3, TCALC_RIGHT_ASSOCIATIVE},
    {{TCALC_UNARY_OPERATOR, "+"}, 3, TCALC_RIGHT_ASSOCIATIVE},
    {{TCALC_UNARY_OPERATOR, "-"}, 3, TCALC_RIGHT_ASSOCIATIVE},
  };

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
          if (tcalc_context_has_binary_func(context, operator_stack[operator_stack_size - 1]->value) == TCALC_OK ||
            tcalc_context_has_unary_func(context, operator_stack[operator_stack_size - 1]->value) == TCALC_OK) {
              if ((err = tcalc_token_clone(operator_stack[operator_stack_size - 1], &rpn_tokens[rpn_tokens_size])) != TCALC_OK) goto cleanup;
              rpn_tokens_size++;
              operator_stack_size--;
          }
        }

        break;
      }
      case TCALC_UNARY_OPERATOR:
      case TCALC_BINARY_OPERATOR: {
        tcalc_op_precedence_t current_optdef, stack_optdef;
        if ((err = tcalc_get_prec_data(OP_PRECEDENCE_DEFS, OP_PRECEDENCE_DEF_COUNT, infix_tokens[i], &current_optdef)) != TCALC_OK) goto cleanup;

        while (operator_stack_size > 0) {
          if (tcalc_get_prec_data(OP_PRECEDENCE_DEFS, OP_PRECEDENCE_DEF_COUNT, operator_stack[operator_stack_size - 1], &stack_optdef) != TCALC_OK) break;
          if (current_optdef.priority > stack_optdef.priority) break;
          if (current_optdef.priority == stack_optdef.priority && current_optdef.associativity == TCALC_RIGHT_ASSOCIATIVE) break;

          if ((err = tcalc_token_clone(operator_stack[operator_stack_size - 1], &rpn_tokens[rpn_tokens_size])) != TCALC_OK) goto cleanup;
          rpn_tokens_size++;
          operator_stack_size--;
        }

        operator_stack[operator_stack_size++] = infix_tokens[i];
        break;
      }
      case TCALC_IDENTIFIER: {  
        if (tcalc_context_has_binary_func(context, infix_tokens[i]->value) == TCALC_OK ||
            tcalc_context_has_unary_func(context, infix_tokens[i]->value) == TCALC_OK) {
          operator_stack[operator_stack_size++] = infix_tokens[i];
        } else if (tcalc_context_has_variable(context, infix_tokens[i]->value) == TCALC_OK) {
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
  #undef OP_PRECEDENCE_DEF_COUNT
}

