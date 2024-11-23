#include "tcalc.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static int32_t tcalc_exprtree_func_list_length(tcalc_exprtree* treeArray, int32_t treeArrayLen, int32_t funcNodeInd)
{
  assert(funcNodeInd >= 0 && funcNodeInd < treeArrayLen);
  assert(treeArray[funcNodeInd].type == TCALC_EXPRTREE_NODE_TYPE_FUNC);

  int32_t length = 0;
  int32_t nodeInd = treeArray[funcNodeInd].as.func.funcArgHeadInd;
  while (nodeInd > 0)
  {
    nodeInd = treeArray[nodeInd].as.funcarg.nextArgInd;
    length++;
  }
  return length;
}


tcalc_err tcalc_eval_exprtree(
  const char* expr, int32_t exprLen,
  tcalc_exprtree* treeArray, int32_t treeArrayLen, int32_t exprNodeInd,
  tcalc_token *tokens, int32_t tokensLen,
  const struct tcalc_ctx* ctx,
  struct tcalc_val* out
) {
  assert(expr != NULL);
  assert(ctx != NULL);
  assert(out != NULL);
  // note that this function not allocate any data in any way

  tcalc_err err = TCALC_ERR_OK;

  assert(exprNodeInd >= 0);
  assert(exprNodeInd < treeArrayLen);
  switch (treeArray[exprNodeInd].type) {
    case TCALC_EXPRTREE_NODE_TYPE_BINARY: {
      const tcalc_exprtree_binary_node binnode = treeArray[exprNodeInd].as.binary;
      tcalc_val operand1 = { 0 }, operand2 = { 0 };
      ret_on_err(
        err,
        tcalc_eval_exprtree(
          expr, exprLen, treeArray, treeArrayLen, binnode.leftTreeInd,
          tokens, tokensLen, ctx, &operand1
        )
      );
      ret_on_err(
        err,
        tcalc_eval_exprtree(
          expr, exprLen, treeArray, treeArrayLen, binnode.rightTreeInd,
          tokens, tokensLen, ctx, &operand2
        )
      );

      if (binnode.tokenIndOImplMult < 0)
      {
        tcalc_binopdef binary_op_def;
        ret_on_err(err, tcalc_ctx_getbinop(ctx, TCALC_STRLIT_PTR_LENI32(""), &binary_op_def));
        out->type = TCALC_VALTYPE_NUM;
        return binary_op_def.func(operand1, operand2, &(out->as.num));
      }

      struct tcalc_token opToken = tokens[binnode.tokenIndOImplMult];
      switch (tokens[binnode.tokenIndOImplMult].type) {
        case TCALC_TOK_BINOP: {
          tcalc_binopdef binary_op_def = { 0 };
          ret_on_err(
            err,
            tcalc_ctx_getbinop(
              ctx,
              tcalc_token_startcp(expr, opToken),
              tcalc_token_len(opToken),
              &binary_op_def
            )
          );

          out->type = TCALC_VALTYPE_NUM;
          return binary_op_def.func(operand1, operand2, &(out->as.num));
        } break;
        case TCALC_TOK_BINLOP: {
          tcalc_binlopdef binary_lop_def;
          ret_on_err(
            err,
            tcalc_ctx_getbinlop(
              ctx,
              tcalc_token_startcp(expr, opToken),
              tcalc_token_len(opToken),
              &binary_lop_def
            )
          );

          out->type = TCALC_VALTYPE_BOOL;
          return binary_lop_def.func(operand1, operand2, &(out->as.boolean));
        } break;
        case TCALC_TOK_RELOP: {
          tcalc_relopdef relopdef;
          ret_on_err(
            err,
            tcalc_ctx_getrelop(
              ctx,
              tcalc_token_startcp(expr, opToken),
              tcalc_token_len(opToken),
              &relopdef
            )
          );

          out->type = TCALC_VALTYPE_BOOL;
          return relopdef.func(operand1, operand2, &(out->as.boolean));
        } break;
        case TCALC_TOK_EQOP: {
          // TODO: Use eqopdef when introduced into tcalc_context struct

          if (operand1.type == TCALC_VALTYPE_NUM && operand2.type == TCALC_VALTYPE_NUM) {
            tcalc_relopdef relopdef;
            ret_on_err(
              err,
              tcalc_ctx_getrelop(
                ctx,
                tcalc_token_startcp(expr, opToken),
                tcalc_token_len(opToken),
                &relopdef
              )
            );
            out->type = TCALC_VALTYPE_BOOL;
            return relopdef.func(operand1, operand2, &(out->as.boolean));
          } else if (operand1.type == TCALC_VALTYPE_BOOL && operand2.type == TCALC_VALTYPE_BOOL) {
            tcalc_binlopdef binlopdef;
            ret_on_err(
              err,
              tcalc_ctx_getbinlop(
                ctx,
                tcalc_token_startcp(expr, opToken),
                tcalc_token_len(opToken),
                &binlopdef
              )
            );
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
      const tcalc_exprtree_unary_node unnode = treeArray[exprNodeInd].as.unary;
      struct tcalc_token opToken = tokens[unnode.tokenInd];
      tcalc_val operand = { 0 };
      ret_on_err(err, tcalc_eval_exprtree(expr, exprLen, treeArray, treeArrayLen, unnode.childTreeInd, tokens, tokensLen, ctx, &operand));

      switch (opToken.type) {
        case TCALC_TOK_UNOP: {
          tcalc_unopdef unary_op_def;
          ret_on_err(err, tcalc_ctx_getunop(ctx, tcalc_token_startcp(expr, opToken), tcalc_token_len(opToken), &unary_op_def));

          out->type = TCALC_VALTYPE_NUM;
          return unary_op_def.func(operand, &(out->as.num));
        } break;
        case TCALC_TOK_UNLOP: {
          tcalc_unlopdef unary_lop_def;
          ret_on_err(err, tcalc_ctx_getunlop(ctx, tcalc_token_startcp(expr, opToken), tcalc_token_len(opToken), &unary_lop_def));

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
      const tcalc_exprtree_value_node value_node = treeArray[exprNodeInd].as.value;
      struct tcalc_token token = tokens[value_node.tokenInd];
      switch (tokens[value_node.tokenInd].type) {
        case TCALC_TOK_ID: {
          if (tcalc_ctx_hasvar(ctx, tcalc_token_startcp(expr, token), tcalc_token_len(token))) {
            tcalc_vardef vardef = { 0 };
            tcalc_ctx_getvar(ctx, tcalc_token_startcp(expr, token), tcalc_token_len(token), &vardef);
            *out = vardef.val;
            return TCALC_ERR_OK;
          }

          // TODO: ERR
          return TCALC_ERR_UNKNOWN_ID;
        } break;
        case TCALC_TOK_NUM: {
          out->type = TCALC_VALTYPE_NUM;
          return tcalc_lpstrtodouble(tcalc_token_startcp(expr, token), tcalc_token_len(token), &(out->as.num));
        } break;
        default: {
          // TODO: ERR
          return TCALC_ERR_INVALID_ARG;
        }
      }
    }
    break;
    case TCALC_EXPRTREE_NODE_TYPE_FUNC: {
      const tcalc_exprtree_func_node funcnode = treeArray[exprNodeInd].as.func;
      const tcalc_token nameToken = tokens[funcnode.tokenInd];
      if (tcalc_ctx_hasunfunc(ctx, tcalc_token_startcp(expr, nameToken), tcalc_token_len(nameToken)))
      {
        const int32_t argListLen = tcalc_exprtree_func_list_length(treeArray, treeArrayLen, exprNodeInd);
        reterr_on_true(err, argListLen != 1, TCALC_ERR_WRONG_ARITY);
        tcalc_val argVal = { 0 };
        ret_on_err(err, tcalc_eval_exprtree(expr, exprLen, treeArray, treeArrayLen, funcnode.funcArgHeadInd, tokens, tokensLen, ctx, &argVal));

        tcalc_unfuncdef unfuncdef = { 0 };
        ret_on_err(err, tcalc_ctx_getunfunc(ctx, tcalc_token_startcp(expr, nameToken), tcalc_token_len(nameToken), &unfuncdef));


        out->type = TCALC_VALTYPE_NUM;
        return unfuncdef.func(argVal, &(out->as.num));
      }
      if (tcalc_ctx_hasbinfunc(ctx, tcalc_token_startcp(expr, nameToken), tcalc_token_len(nameToken)))
      {
        const int32_t argListLen = tcalc_exprtree_func_list_length(treeArray, treeArrayLen, exprNodeInd);
        reterr_on_true(err, argListLen != 2, TCALC_ERR_WRONG_ARITY);
        tcalc_val argVal1 = { 0 };
        ret_on_err(err, tcalc_eval_exprtree(expr, exprLen, treeArray, treeArrayLen, funcnode.funcArgHeadInd, tokens, tokensLen, ctx, &argVal1));
        tcalc_val argVal2 = { 0 };
        ret_on_err(err, tcalc_eval_exprtree(expr, exprLen, treeArray, treeArrayLen, treeArray[funcnode.funcArgHeadInd].as.funcarg.nextArgInd, tokens, tokensLen, ctx, &argVal2));

        tcalc_binfuncdef binfuncdef;
        ret_on_err(err, tcalc_ctx_getbinfunc(ctx, tcalc_token_startcp(expr, nameToken), tcalc_token_len(nameToken), &binfuncdef));

        out->type = TCALC_VALTYPE_NUM;
        return binfuncdef.func(argVal1, argVal2, &(out->as.num));
      }
      else
      {
        // TODO: Better err
        return TCALC_ERR_UNKNOWN_ID;
      }
    }
    break;
    case TCALC_EXPRTREE_NODE_TYPE_FUNCARG: {
      return tcalc_eval_exprtree(expr, exprLen, treeArray, treeArrayLen, treeArray[exprNodeInd].as.funcarg.exprInd, tokens, tokensLen, ctx, out);
    }
    break;
  }

  assert(0); // unreachable
  return TCALC_ERR_INVALID_ARG;
}
