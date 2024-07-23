#include "tcalc_exprtree.h"

#include "tcalc_context.h"
#include "tcalc_string.h"
#include "tcalc_func.h"
#include "tcalc_val.h"
#include "tcalc_val_func.h"
#include "tcalc_mem.h"
#include "tcalc_tokens.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


#if 0
// TODO: Remove legacy code
static tcalc_err tcalc_rpn_tokens_to_exprtree(tcalc_token** tokens, size_t nb_tokens, const tcalc_ctx* ctx, tcalc_exprtree** out);
#endif

#if 0
// TODO: Remove legacy code
tcalc_err tcalc_create_exprtree_rpn(const char* rpn, const tcalc_ctx* ctx, tcalc_exprtree** out) {
  tcalc_token** tokens;
  size_t nb_tokens;
  tcalc_err err = tcalc_tokenize_rpn(rpn, &tokens, &nb_tokens);
  if (err) return err;

  err = tcalc_rpn_tokens_to_exprtree(tokens, nb_tokens, ctx, out);
  TCALC_ARR_FREE_F(tokens, nb_tokens, tcalc_token_free);
  return err;
}
#endif

int tcalc_exprtree_is_vardef(tcalc_exprtree* expr) {
  if (expr == NULL) return 0;

  // if (expr->token->type == TCALC_TOK_RELOP) {}
  return 1;
}


/**
 * Detailed explanation of how an expression tree is evaluated
 *
 * If the token type is  number:
 *  - simply read the number value in the
 *
 * If the token type is a unary operator
 * a
*/
tcalc_err tcalc_eval_exprtree(tcalc_exprtree* expr, const tcalc_ctx* ctx, tcalc_val* out) {
  assert(expr != NULL);
  assert(ctx != NULL);
  assert(out != NULL);
  // note that this function not allocate any data in any way

  tcalc_err err = TCALC_ERR_OK;

  switch (expr->type) {
    case TCALC_EXPRTREE_NODE_TYPE_BINARY: {
      const tcalc_exprtree_binary_node binnode = expr->as.binary;
      assert(binnode.left != NULL);
      assert(binnode.right != NULL);

      switch (binnode.token->type) {
        case TCALC_TOK_ID: {
          if (tcalc_ctx_hasbinfunc(ctx, binnode.token->val)) {
            tcalc_binfuncdef binary_func_def;
            tcalc_ctx_getbinfunc(ctx, binnode.token->val, &binary_func_def);

            tcalc_val operand1, operand2;
            ret_on_err(err, tcalc_eval_exprtree(binnode.left, ctx, &operand1));
            ret_on_err(err, tcalc_eval_exprtree(binnode.right, ctx, &operand2));

            out->type = TCALC_VALTYPE_NUM;
            return binary_func_def.func(operand1, operand2, &(out->as.num));
          }
          // TODO: More descriptive error
          return TCALC_ERR_UNKNOWN_ID;
        } break;
        case TCALC_TOK_BINOP: {
          tcalc_val operand1, operand2;
          tcalc_binopdef binary_op_def;
          ret_on_err(err, tcalc_ctx_getbinop(ctx, binnode.token->val, &binary_op_def));
          ret_on_err(err, tcalc_eval_exprtree(binnode.left, ctx, &operand1));
          ret_on_err(err, tcalc_eval_exprtree(binnode.right, ctx, &operand2));

          out->type = TCALC_VALTYPE_NUM;
          return binary_op_def.func(operand1, operand2, &(out->as.num));
        } break;
        case TCALC_TOK_BINLOP: {
          tcalc_val operand1, operand2;
          tcalc_binlopdef binary_lop_def;
          ret_on_err(err, tcalc_ctx_getbinlop(ctx, binnode.token->val, &binary_lop_def));
          ret_on_err(err, tcalc_eval_exprtree(binnode.left, ctx, &operand1));
          ret_on_err(err, tcalc_eval_exprtree(binnode.right, ctx, &operand2));

          out->type = TCALC_VALTYPE_BOOL;
          return binary_lop_def.func(operand1, operand2, &(out->as.boolean));
        } break;
        case TCALC_TOK_RELOP: {
          tcalc_val operand1, operand2;
          tcalc_relopdef relopdef;
          ret_on_err(err, tcalc_ctx_getrelop(ctx, binnode.token->val, &relopdef));
          ret_on_err(err, tcalc_eval_exprtree(binnode.left, ctx, &operand1));
          ret_on_err(err, tcalc_eval_exprtree(binnode.right, ctx, &operand2));

          out->type = TCALC_VALTYPE_BOOL;
          return relopdef.func(operand1, operand2, &(out->as.boolean));
        } break;
        case TCALC_TOK_EQOP: {
          // TODO: Use eqopdef when introduced into tcalc_context struct
          tcalc_val operand1, operand2;
          ret_on_err(err, tcalc_eval_exprtree(binnode.left, ctx, &operand1));
          ret_on_err(err, tcalc_eval_exprtree(binnode.right, ctx, &operand2));

          if (operand1.type == TCALC_VALTYPE_NUM && operand2.type == TCALC_VALTYPE_NUM) {
            tcalc_relopdef relopdef;
            ret_on_err(err, tcalc_ctx_getrelop(ctx, binnode.token->val, &relopdef));
            out->type = TCALC_VALTYPE_BOOL;
            return relopdef.func(operand1, operand2, &(out->as.boolean));
          } else if (operand1.type == TCALC_VALTYPE_BOOL && operand2.type == TCALC_VALTYPE_BOOL) {
            tcalc_binlopdef binlopdef;
            ret_on_err(err, tcalc_ctx_getbinlop(ctx, binnode.token->val, &binlopdef));
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
      const tcalc_exprtree_unary_node unnode = expr->as.unary;
      assert(unnode.child != NULL);

      switch (unnode.token->type) {
        case TCALC_TOK_ID: {
          if (tcalc_ctx_hasunfunc(ctx, unnode.token->val)) {
            tcalc_unfuncdef unary_func_def;
            tcalc_ctx_getunfunc(ctx, unnode.token->val, &unary_func_def);

            tcalc_val operand;
            ret_on_err(err, tcalc_eval_exprtree(unnode.child, ctx, &operand));

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
          ret_on_err(err, tcalc_ctx_getunop(ctx, unnode.token->val, &unary_op_def));
          ret_on_err(err, tcalc_eval_exprtree(unnode.child, ctx, &operand));

          out->type = TCALC_VALTYPE_NUM;
          return unary_op_def.func(operand, &(out->as.num));
        } break;
        case TCALC_TOK_UNLOP: {
          tcalc_val operand;
          tcalc_unlopdef unary_lop_def;
          ret_on_err(err, tcalc_ctx_getunlop(ctx, unnode.token->val, &unary_lop_def));
          ret_on_err(err, tcalc_eval_exprtree(unnode.child, ctx, &operand));

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
      const tcalc_exprtree_value_node value_node = expr->as.value;
      switch (value_node.token->type) {
        case TCALC_TOK_ID: {
          if (tcalc_ctx_hasvar(ctx, value_node.token->val)) {
            tcalc_vardef vardef;
            tcalc_ctx_getvar(ctx, value_node.token->val, &vardef);
            *out = vardef.val;
            return TCALC_ERR_OK;
          }

          // TODO: ERR
          return TCALC_ERR_UNKNOWN_ID;
        } break;
        case TCALC_TOK_NUM: {
          out->type = TCALC_VALTYPE_NUM;
          return tcalc_strtodouble(value_node.token->val, &(out->as.num));
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
      tcalc_token_free(head->as.binary.token);
    } break;
    case TCALC_EXPRTREE_NODE_TYPE_UNARY: {
      tcalc_exprtree_free(head->as.unary.child);
      tcalc_token_free(head->as.binary.token);
    } break;
    case TCALC_EXPRTREE_NODE_TYPE_VALUE: {
      tcalc_token_free(head->as.value.token);
    } break;
  }
  free(head);
}

void tcalc_exprtree_free_children(tcalc_exprtree* head) {
  if (head == NULL) return;
  switch (head->type) {
    case TCALC_EXPRTREE_NODE_TYPE_BINARY:
      tcalc_exprtree_free(head->as.binary.left);
      head->as.binary.left = NULL;
      tcalc_exprtree_free(head->as.binary.right);
      head->as.binary.right = NULL;
      break;
    case TCALC_EXPRTREE_NODE_TYPE_UNARY:
      tcalc_exprtree_free(head->as.unary.child);
      head->as.unary.child = NULL;
      break;
    case TCALC_EXPRTREE_NODE_TYPE_VALUE:
      // no-op
      break;
  }
}


#if 0
// TODO: Remove legacy code
static tcalc_err tcalc_rpn_tokens_to_exprtree(tcalc_token** tokens, size_t nb_tokens, const tcalc_ctx* ctx, tcalc_exprtree** out) {
  tcalc_err err = TCALC_ERR_OK;

  tcalc_exprtree** tree_stack = (tcalc_exprtree**)malloc(sizeof(tcalc_exprtree*) * nb_tokens);
  if (tree_stack == NULL) return TCALC_ERR_BAD_ALLOC;
  size_t tree_stack_size = 0;

  for (size_t i = 0; i < nb_tokens; i++) {
    switch (tokens[i]->type) {
      case TCALC_TOK_NUM: {
        tcalc_exprtree* tree_node;
        cleanup_on_err(err, tcalc_exprtree_node_alloc(tokens[i], 0, &tree_node));
        tree_stack[tree_stack_size++] = tree_node;
        break;
      } // TCALC_TOK_NUM
      case TCALC_TOK_BINOP: {
        cleanup_on_err(err, err_pred(tree_stack_size < 2, TCALC_ERR_INVALID_OP));

        tcalc_exprtree* tree_node;
        cleanup_on_err(err, tcalc_exprtree_node_alloc(tokens[i], 2, &tree_node));

        tree_node->children[0] = tree_stack[tree_stack_size - 2];
        tree_node->children[1] = tree_stack[tree_stack_size - 1];

        tree_stack[tree_stack_size - 2] = tree_node;
        tree_stack_size--;
        break;
      } // TCALC_TOK_BINOP
      case TCALC_TOK_UNOP: {
        cleanup_on_err(err, err_pred(tree_stack_size < 1, TCALC_ERR_INVALID_OP));

        tcalc_exprtree* tree_node;
        cleanup_on_err(err, tcalc_exprtree_node_alloc(tokens[i], 1, &tree_node));

        tree_node->children[0] = tree_stack[tree_stack_size - 1];
        tree_stack[tree_stack_size - 1] = tree_node;
        break;
      } // TCALC_TOK_UNOP
      case TCALC_TOK_ID: {

        if (tcalc_ctx_hasvar(ctx, tokens[i]->val)) {

          tcalc_exprtree* tree_node;
          cleanup_on_err(err, tcalc_exprtree_node_alloc(tokens[i], 0, &tree_node));
          tree_stack[tree_stack_size++] = tree_node;

        } else if (tcalc_ctx_hasbinfunc(ctx, tokens[i]->val)) {
          cleanup_on_err(err, tree_stack_size < 2 ? TCALC_ERR_INVALID_OP : TCALC_ERR_OK);

          tcalc_exprtree* tree_node;
          cleanup_on_err(err, tcalc_exprtree_node_alloc(tokens[i], 2, &tree_node));

          tree_node->children[0] = tree_stack[tree_stack_size - 2];
          tree_node->children[1] = tree_stack[tree_stack_size - 1];

          tree_stack[tree_stack_size - 2] = tree_node;
          tree_stack_size--;
        } else if (tcalc_ctx_hasunfunc(ctx, tokens[i]->val)) {
          cleanup_on_err(err, tree_stack_size < 1 ? TCALC_ERR_INVALID_OP : TCALC_ERR_OK);

          tcalc_exprtree* tree_node;
          cleanup_on_err(err, tcalc_exprtree_node_alloc(tokens[i], 1, &tree_node));

          tree_node->children[0] = tree_stack[tree_stack_size - 1];
          tree_stack[tree_stack_size - 1] = tree_node;
        } else {
          err = TCALC_ERR_UNKNOWN_ID;
          goto cleanup;
        }

        break;
      } // TCALC_TOK_ID
      default: goto cleanup;
    }
  }

  err = TCALC_ERR_OK;
  if (tree_stack_size == 1) {
    *out = tree_stack[0];
  } else {
    err = TCALC_ERR_INVALID_ARG;
    goto cleanup;
  }

  free(tree_stack);
  return err;

  cleanup:
    TCALC_ARR_FREE_F(tree_stack, tree_stack_size, tcalc_exprtree_node_free);
    return err;
}
#endif

#if 0
// TODO: Remove all of this legacy code
tcalc_err tcalc_exprtree_node_alloc(tcalc_token* token, size_t nb_children, tcalc_exprtree** out) {
  tcalc_err err;

  tcalc_exprtree* node = (tcalc_exprtree*)calloc(1, sizeof(tcalc_exprtree));
  if (node == NULL) return TCALC_ERR_BAD_ALLOC;

  if ((err = tcalc_token_clone(token, &node->token)) != TCALC_ERR_OK) {
    free(node);
    return err;
  }

  node->children = NULL;
  node->nb_children = nb_children;

  if (nb_children > 0) {
    node->children = (tcalc_exprtree**)calloc(nb_children, sizeof(tcalc_exprtree*));
    if (node->children == NULL) {
      tcalc_token_free(node->token);
      free(node);
      return TCALC_ERR_BAD_ALLOC;
    }
  }

  *out = node;
  return TCALC_ERR_OK;
}

void tcalc_exprtree_node_free(tcalc_exprtree* node) {
  if (node == NULL) return;
  tcalc_token_free(node->token);
  free(node->children);
  free(node);
}
#endif