#include "tcalc_context.h"

#include "tcalc_tokens.h"
#include "tcalc_error.h"
#include "tcalc_mem.h"

#include <stdlib.h>
#include <string.h>

tcalc_err tcalc_ctx_get_token_op_data(const tcalc_ctx* ctx, tcalc_token* token, tcalc_opdata* out) {
  tcalc_err err = TCALC_OK;

  switch (token->type) {
    case TCALC_UNARY_OPERATOR: {
      tcalc_unopdef* unary_op_def;
      ret_on_err(err, tcalc_ctx_getunop(ctx, token->val, &unary_op_def));
      *out = tcalc_getunopdata(unary_op_def);
      return TCALC_OK;
    }
    case TCALC_BINARY_OPERATOR: {
      tcalc_binopdef* binary_op_def;
      ret_on_err(err, tcalc_ctx_getbinop(ctx, token->val, &binary_op_def));
      *out = tcalc_getbinopdata(binary_op_def);
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
tcalc_err tcalc_infix_tokens_to_rpn_tokens(tcalc_token** infix_toks, size_t nb_infix_toks, const tcalc_ctx* ctx, tcalc_token*** out, size_t* out_size) {
  tcalc_err err = TCALC_OK;

  tcalc_token** opstk = (tcalc_token**)malloc(sizeof(tcalc_token*) * nb_infix_toks);
  if (opstk == NULL) return TCALC_BAD_ALLOC;
  size_t opstk_size = 0;

  tcalc_token** rpn_toks = (tcalc_token**)malloc(sizeof(tcalc_token*) * nb_infix_toks); 
  size_t rpn_toks_size = 0;
  if (rpn_toks == NULL) {
    free(opstk);
    return TCALC_BAD_ALLOC;
  }

  for (size_t i = 0; i < nb_infix_toks; i++) {
    switch (infix_toks[i]->type) {
      case TCALC_NUMBER: {
        cleanup_on_err(err, tcalc_token_clone(infix_toks[i], &rpn_toks[rpn_toks_size]));
        rpn_toks_size++;
        break;
      }
      case TCALC_GROUP_START: { // "(" or "["
        opstk[opstk_size++] = infix_toks[i];
        break;
      }
      case TCALC_GROUP_END: { // ")" or "]"
        cleanup_on_err(err, err_pred(opstk_size == 0, TCALC_INVALID_OP));

        const char* opener;
        if (strcmp(infix_toks[i]->val, ")") == 0) {
          opener = "(";
        } else if (strcmp(infix_toks[i]->val, "]") == 0) {
          opener = "[";
        } else {
          err = TCALC_INVALID_OP;
          goto cleanup;
        }

        while (strcmp(opstk[opstk_size - 1]->val, opener) != 0) { // keep popping onto output until the opening parenthesis is found
          cleanup_on_err(err, tcalc_token_clone(opstk[opstk_size - 1], &rpn_toks[rpn_toks_size]));
          rpn_toks_size++;
          opstk_size--;
          
          cleanup_on_err(err, err_pred(opstk_size == 0, TCALC_INVALID_OP));
        }
        opstk_size--; // pop off opening grouping symbol

        if (opstk_size >= 1) {
          if (tcalc_ctx_hasfunc(ctx, opstk[opstk_size - 1]->val)) {
              cleanup_on_err(err, tcalc_token_clone(opstk[opstk_size - 1], &rpn_toks[rpn_toks_size]));
              rpn_toks_size++;
              opstk_size--;
          }
        }

        break;
      }
      case TCALC_UNARY_OPERATOR:
      case TCALC_BINARY_OPERATOR: {
        tcalc_opdata curr_opdata, stk_opdata;
        cleanup_on_err(err, tcalc_ctx_get_token_op_data(ctx, infix_toks[i], &curr_opdata));

        while (opstk_size > 0) {
          tcalc_token* stack_top = opstk[opstk_size - 1];
          if (stack_top->type == TCALC_GROUP_START) break;

          cleanup_on_err(err, tcalc_ctx_get_token_op_data(ctx, stack_top, &stk_opdata));
          if (curr_opdata.prec > stk_opdata.prec) break;
          if (curr_opdata.prec == stk_opdata.prec && curr_opdata.assoc == TCALC_RIGHT_ASSOC) break;

          cleanup_on_err(err, tcalc_token_clone(opstk[opstk_size - 1], &rpn_toks[rpn_toks_size]));
          rpn_toks_size++;
          opstk_size--;
        }

        opstk[opstk_size++] = infix_toks[i];
        break;
      }
      case TCALC_IDENTIFIER: {  
        if (tcalc_ctx_hasfunc(ctx, infix_toks[i]->val)) {
          opstk[opstk_size++] = infix_toks[i];
        } else if (tcalc_ctx_hasvar(ctx, infix_toks[i]->val)) {
          cleanup_on_err(err, tcalc_token_clone(infix_toks[i], &rpn_toks[rpn_toks_size]));
          rpn_toks_size++;
        } else {
          err = TCALC_UNKNOWN_IDENTIFIER;
          goto cleanup;
        }

        break;
      }
      case TCALC_PARAM_SEPARATOR: {
        cleanup_on_err(err, err_pred(opstk_size == 0, TCALC_INVALID_OP));

        while (opstk[opstk_size - 1]->type != TCALC_GROUP_START) { // keep popping onto output until the opening grouping symbol is found
          cleanup_on_err(err, tcalc_token_clone(opstk[opstk_size - 1], &rpn_toks[rpn_toks_size]));
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
    cleanup_on_err(err, tcalc_token_clone(opstk[opstk_size - 1], &rpn_toks[rpn_toks_size]));
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

