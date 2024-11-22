#include "tcalc_exprtree.h"

#include "tcalc_context.h"
#include "tcalc_string.h"
#include "tcalc_func.h"
#include "tcalc_val.h"
#include "tcalc_val_func.h"
#include "tcalc_mem.h"
#include "tcalc_tokens.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


/**
 * Detailed explanation of how an expression tree is evaluated
 *
 * If the token type is  number:
 *  - simply read the number value in the
 *
 * If the token type is a unary operator
 * a
*/
tcalc_err tcalc_eval_exprtree(const char* expr, tcalc_exprtree* exprtree, const tcalc_ctx* ctx, tcalc_val* out) {
  assert(expr != NULL);
  assert(ctx != NULL);
  assert(out != NULL);
  // note that this function not allocate any data in any way

  tcalc_err err = TCALC_ERR_OK;

  switch (exprtree->type) {
    case TCALC_EXPRTREE_NODE_TYPE_BINARY: {
      const tcalc_exprtree_binary_node binnode = exprtree->as.binary;
      assert(binnode.left != NULL);
      assert(binnode.right != NULL);

      switch (binnode.token.type) {
        case TCALC_TOK_ID: {
          if (tcalc_ctx_hasbinfunc(ctx, tcalc_token_startcp(expr, binnode.token), tcalc_token_len(binnode.token))) {
            tcalc_binfuncdef binary_func_def;
            tcalc_ctx_getbinfunc(ctx, tcalc_token_startcp(expr, binnode.token), tcalc_token_len(binnode.token), &binary_func_def);

            tcalc_val operand1, operand2;
            ret_on_err(err, tcalc_eval_exprtree(expr, binnode.left, ctx, &operand1));
            ret_on_err(err, tcalc_eval_exprtree(expr, binnode.right, ctx, &operand2));

            out->type = TCALC_VALTYPE_NUM;
            return binary_func_def.func(operand1, operand2, &(out->as.num));
          }
          // TODO: More descriptive error
          return TCALC_ERR_UNKNOWN_ID;
        } break;
        case TCALC_TOK_BINOP: {
          tcalc_val operand1, operand2;
          tcalc_binopdef binary_op_def;
          ret_on_err(err, tcalc_ctx_getbinop(ctx, tcalc_token_startcp(expr, binnode.token), tcalc_token_len(binnode.token), &binary_op_def));
          ret_on_err(err, tcalc_eval_exprtree(expr, binnode.left, ctx, &operand1));
          ret_on_err(err, tcalc_eval_exprtree(expr, binnode.right, ctx, &operand2));

          out->type = TCALC_VALTYPE_NUM;
          return binary_op_def.func(operand1, operand2, &(out->as.num));
        } break;
        case TCALC_TOK_BINLOP: {
          tcalc_val operand1, operand2;
          tcalc_binlopdef binary_lop_def;
          ret_on_err(err, tcalc_ctx_getbinlop(ctx, tcalc_token_startcp(expr, binnode.token), tcalc_token_len(binnode.token), &binary_lop_def));
          ret_on_err(err, tcalc_eval_exprtree(expr, binnode.left, ctx, &operand1));
          ret_on_err(err, tcalc_eval_exprtree(expr, binnode.right, ctx, &operand2));

          out->type = TCALC_VALTYPE_BOOL;
          return binary_lop_def.func(operand1, operand2, &(out->as.boolean));
        } break;
        case TCALC_TOK_RELOP: {
          tcalc_val operand1, operand2;
          tcalc_relopdef relopdef;
          ret_on_err(err, tcalc_ctx_getrelop(ctx, tcalc_token_startcp(expr, binnode.token), tcalc_token_len(binnode.token), &relopdef));
          ret_on_err(err, tcalc_eval_exprtree(expr, binnode.left, ctx, &operand1));
          ret_on_err(err, tcalc_eval_exprtree(expr, binnode.right, ctx, &operand2));

          out->type = TCALC_VALTYPE_BOOL;
          return relopdef.func(operand1, operand2, &(out->as.boolean));
        } break;
        case TCALC_TOK_EQOP: {
          // TODO: Use eqopdef when introduced into tcalc_context struct
          tcalc_val operand1, operand2;
          ret_on_err(err, tcalc_eval_exprtree(expr, binnode.left, ctx, &operand1));
          ret_on_err(err, tcalc_eval_exprtree(expr, binnode.right, ctx, &operand2));

          if (operand1.type == TCALC_VALTYPE_NUM && operand2.type == TCALC_VALTYPE_NUM) {
            tcalc_relopdef relopdef;
            ret_on_err(err, tcalc_ctx_getrelop(ctx, tcalc_token_startcp(expr, binnode.token), tcalc_token_len(binnode.token), &relopdef));
            out->type = TCALC_VALTYPE_BOOL;
            return relopdef.func(operand1, operand2, &(out->as.boolean));
          } else if (operand1.type == TCALC_VALTYPE_BOOL && operand2.type == TCALC_VALTYPE_BOOL) {
            tcalc_binlopdef binlopdef;
            ret_on_err(err, tcalc_ctx_getbinlop(ctx, tcalc_token_startcp(expr, binnode.token), tcalc_token_len(binnode.token), &binlopdef));
            out->type = TCALC_VALTYPE_BOOL;
            return binlopdef.func(operand1, operand2, &(out->as.boolean));
          }

          return TCALC_ERR_BAD_CAST;
        } break;
        default: {
          return TCALC_ERR_INVALID_ARG;
        }
      }

    } break;
    case TCALC_EXPRTREE_NODE_TYPE_UNARY: {
      const tcalc_exprtree_unary_node unnode = exprtree->as.unary;
      assert(unnode.child != NULL);

      switch (unnode.token.type) {
        case TCALC_TOK_ID: {
          if (tcalc_ctx_hasunfunc(ctx, tcalc_token_startcp(expr, unnode.token), tcalc_token_len(unnode.token))) {
            tcalc_unfuncdef unary_func_def;
            tcalc_ctx_getunfunc(ctx, tcalc_token_startcp(expr, unnode.token), tcalc_token_len(unnode.token), &unary_func_def);

            tcalc_val operand;
            ret_on_err(err, tcalc_eval_exprtree(expr, unnode.child, ctx, &operand));

            out->type = TCALC_VALTYPE_NUM;
            return unary_func_def.func(operand, &(out->as.num));
          } else {
            // TOOD: Err
            return TCALC_ERR_INVALID_ARG;
          }
        } break;
        case TCALC_TOK_UNOP: {
          tcalc_val operand;
          tcalc_unopdef unary_op_def;
          ret_on_err(err, tcalc_ctx_getunop(ctx, tcalc_token_startcp(expr, unnode.token), tcalc_token_len(unnode.token), &unary_op_def));
          ret_on_err(err, tcalc_eval_exprtree(expr, unnode.child, ctx, &operand));

          out->type = TCALC_VALTYPE_NUM;
          return unary_op_def.func(operand, &(out->as.num));
        } break;
        case TCALC_TOK_UNLOP: {
          tcalc_val operand;
          tcalc_unlopdef unary_lop_def;
          ret_on_err(err, tcalc_ctx_getunlop(ctx, tcalc_token_startcp(expr, unnode.token), tcalc_token_len(unnode.token), &unary_lop_def));
          ret_on_err(err, tcalc_eval_exprtree(expr, unnode.child, ctx, &operand));

          out->type = TCALC_VALTYPE_BOOL;
          return unary_lop_def.func(operand, &(out->as.boolean));
        } break;
        default: {
          // TODO: More descriptive error
          return TCALC_ERR_INVALID_ARG;
        }
      }
    } break;
    case TCALC_EXPRTREE_NODE_TYPE_VALUE: {
      const tcalc_exprtree_value_node value_node = exprtree->as.value;
      switch (value_node.token.type) {
        case TCALC_TOK_ID: {
          if (tcalc_ctx_hasvar(ctx, tcalc_token_startcp(expr, value_node.token), tcalc_token_len(value_node.token))) {
            tcalc_vardef vardef;
            tcalc_ctx_getvar(ctx, tcalc_token_startcp(expr, value_node.token), tcalc_token_len(value_node.token), &vardef);
            *out = vardef.val;
            return TCALC_ERR_OK;
          }

          // TODO: ERR
          return TCALC_ERR_UNKNOWN_ID;
        } break;
        case TCALC_TOK_NUM: {
          out->type = TCALC_VALTYPE_NUM;
          return tcalc_lpstrtodouble(tcalc_token_startcp(expr, value_node.token), tcalc_token_len(value_node.token), &(out->as.num));
        } break;
        default: {
          // TODO: ERR
          return TCALC_ERR_INVALID_ARG;
        }
      }
    }
  }

  assert(0); // unreachable
  return TCALC_ERR_INVALID_ARG;
}

void tcalc_exprtree_free(tcalc_exprtree* head) {
  if (head == NULL) return;
  switch (head->type) {
    case TCALC_EXPRTREE_NODE_TYPE_BINARY: {
      tcalc_exprtree_free(head->as.binary.left);
      tcalc_exprtree_free(head->as.binary.right);
    } break;
    case TCALC_EXPRTREE_NODE_TYPE_UNARY: {
      tcalc_exprtree_free(head->as.unary.child);
    } break;
    case TCALC_EXPRTREE_NODE_TYPE_VALUE: {
      // do nothing
    } break;
  }
  free(head);
}

void tcalc_exprtree_free_children(tcalc_exprtree* head) {
  if (head == NULL) return;
  switch (head->type) {
    case TCALC_EXPRTREE_NODE_TYPE_BINARY: {
      tcalc_exprtree_free(head->as.binary.left);
      tcalc_exprtree_free(head->as.binary.right);
      head->as.binary.left = NULL;
      head->as.binary.right = NULL;
    } break;
    case TCALC_EXPRTREE_NODE_TYPE_UNARY: {
      tcalc_exprtree_free(head->as.unary.child);
      head->as.unary.child = NULL;
    } break;
    case TCALC_EXPRTREE_NODE_TYPE_VALUE: {
      // no-op
    } break;
  }
}
