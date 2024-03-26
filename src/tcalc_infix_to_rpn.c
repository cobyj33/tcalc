#include "tcalc_context.h"
#include "tcalc_tokens.h"
#include "tcalc_error.h"
#include "tcalc_mem.h"

#include <stdlib.h>
#include <string.h>

tcalc_error_t tcalc_context_get_token_op_data(const tcalc_context_t* ctx, tcalc_token_t* token, tcalc_op_data_t* out) {
  tcalc_error_t err = TCALC_OK;

  switch (token->type) {
    case TCALC_UNARY_OPERATOR: {
      tcalc_unary_op_def_t* unary_op_def;
      if ((err = tcalc_context_get_unary_op(ctx, token->value, &unary_op_def)) != TCALC_OK) return err;
      *out = tcalc_unary_op_get_data(unary_op_def);
      return TCALC_OK;
    }
    case TCALC_BINARY_OPERATOR: {
      tcalc_binary_op_def_t* binary_op_def;
      if ((err = tcalc_context_get_binary_op(ctx, token->value, &binary_op_def)) != TCALC_OK) return err;
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
tcalc_error_t tcalc_infix_tokens_to_rpn_tokens(tcalc_token_t** infix_toks, size_t nb_infix_toks, const tcalc_context_t* ctx, tcalc_token_t*** out, size_t* out_size) {
  tcalc_error_t err = TCALC_OK;

  tcalc_token_t** opstk = (tcalc_token_t**)malloc(sizeof(tcalc_token_t*) * nb_infix_toks);
  if (opstk == NULL) return TCALC_BAD_ALLOC;
  size_t opstk_size = 0;

  tcalc_token_t** rpn_toks = (tcalc_token_t**)malloc(sizeof(tcalc_token_t*) * nb_infix_toks); // tcalc_token_t* array, will be joined
  size_t rpn_toks_size = 0;
  if (rpn_toks == NULL) {
    free(opstk);
    return TCALC_BAD_ALLOC;
  }

  for (size_t i = 0; i < nb_infix_toks; i++) {
    switch (infix_toks[i]->type) {
      case TCALC_NUMBER: {
        if ((err = tcalc_token_clone(infix_toks[i], &rpn_toks[rpn_toks_size])) != TCALC_OK) goto cleanup;
        rpn_toks_size++;
        break;
      }
      case TCALC_GROUP_START: { // "(" or "["
        opstk[opstk_size++] = infix_toks[i];
        break;
      }
      case TCALC_GROUP_END: { // ")" or "]"
        if (opstk_size == 0) {
          err = TCALC_INVALID_OP;
          goto cleanup;
        }

        const char* opener;
        if (strcmp(infix_toks[i]->value, ")") == 0) {
          opener = "(";
        } else if (strcmp(infix_toks[i]->value, "]") == 0) {
          opener = "[";
        } else {
          err = TCALC_INVALID_OP;
          goto cleanup;
        }

        while (strcmp(opstk[opstk_size - 1]->value, opener) != 0) { // keep popping onto output until the opening parenthesis is found
          if ((err = tcalc_token_clone(opstk[opstk_size - 1], &rpn_toks[rpn_toks_size])) != TCALC_OK) goto cleanup;
          rpn_toks_size++;
          opstk_size--;
          
          if (opstk_size == 0) {
            err = TCALC_INVALID_OP;
            goto cleanup;
          }
        }
        opstk_size--; // pop off opening grouping symbol

        if (opstk_size >= 1) {
          if (tcalc_context_has_func(ctx, opstk[opstk_size - 1]->value)) {
              if ((err = tcalc_token_clone(opstk[opstk_size - 1], &rpn_toks[rpn_toks_size])) != TCALC_OK) goto cleanup;
              rpn_toks_size++;
              opstk_size--;
          }
        }

        break;
      }
      case TCALC_UNARY_OPERATOR:
      case TCALC_BINARY_OPERATOR: {
        tcalc_op_data_t curr_opdata, stk_opdata;
        if ((err = tcalc_context_get_token_op_data(ctx, infix_toks[i], &curr_opdata))) goto cleanup;

        while (opstk_size > 0) {
          tcalc_token_t* stack_top = opstk[opstk_size - 1];
          if (stack_top->type == TCALC_GROUP_START) break;

          if ((err = tcalc_context_get_token_op_data(ctx, stack_top, &stk_opdata))) goto cleanup;
          if (curr_opdata.precedence > stk_opdata.precedence) break;
          if (curr_opdata.precedence == stk_opdata.precedence && curr_opdata.associativity == TCALC_RIGHT_ASSOCIATIVE) break;

          if ((err = tcalc_token_clone(opstk[opstk_size - 1], &rpn_toks[rpn_toks_size])) != TCALC_OK) goto cleanup;
          rpn_toks_size++;
          opstk_size--;
        }

        opstk[opstk_size++] = infix_toks[i];
        break;
      }
      case TCALC_IDENTIFIER: {  
        if (tcalc_context_has_func(ctx, infix_toks[i]->value)) {
          opstk[opstk_size++] = infix_toks[i];
        } else if (tcalc_context_has_variable(ctx, infix_toks[i]->value)) {
          if ((err = tcalc_token_clone(infix_toks[i], &rpn_toks[rpn_toks_size])) != TCALC_OK) goto cleanup;
          rpn_toks_size++;
        } else {
          err = TCALC_UNKNOWN_IDENTIFIER;
          goto cleanup;
        }

        break;
      }
      case TCALC_PARAM_SEPARATOR: {
        if (opstk_size == 0) {
          err = TCALC_INVALID_OP;
          goto cleanup;
        }

        while (opstk[opstk_size - 1]->type != TCALC_GROUP_START) { // keep popping onto output until the opening grouping symbol is found
          if ((err = tcalc_token_clone(opstk[opstk_size - 1], &rpn_toks[rpn_toks_size])) != TCALC_OK) goto cleanup;
          rpn_toks_size++;
          opstk_size--;
        }

        break;
      }
      default: {
        err = TCALC_UNIMPLEMENTED;
        goto cleanup;
      }
    }
  }

  while (opstk_size > 0) {
    if ((err = tcalc_token_clone(opstk[opstk_size - 1], &rpn_toks[rpn_toks_size])) != TCALC_OK) goto cleanup;
    rpn_toks_size++;
    opstk_size--;
  }

  free(opstk);
  *out = rpn_toks;
  *out_size = rpn_toks_size;
  return TCALC_OK;

  cleanup:
    free(opstk);
    TCALC_ARR_FREE_F(rpn_toks, rpn_toks_size, tcalc_token_free);
    *out_size = 0;
    return err;
}

