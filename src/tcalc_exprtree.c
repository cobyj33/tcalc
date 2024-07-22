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


static tcalc_err tcalc_rpn_tokens_to_exprtree(tcalc_token** tokens, size_t nb_tokens, const tcalc_ctx* ctx, tcalc_exprtree** out);

tcalc_err tcalc_create_exprtree_rpn(const char* rpn, const tcalc_ctx* ctx, tcalc_exprtree** out) {
  tcalc_token** tokens;
  size_t nb_tokens;
  tcalc_err err = tcalc_tokenize_rpn(rpn, &tokens, &nb_tokens);
  if (err) return err;

  err = tcalc_rpn_tokens_to_exprtree(tokens, nb_tokens, ctx, out);
  TCALC_ARR_FREE_F(tokens, nb_tokens, tcalc_token_free);
  return err;
}

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
  // note that this function not allocate any data in any way

  tcalc_err err = TCALC_ERR_OK;

  switch (expr->token->type) {
    case TCALC_TOK_NUM: {
      out->type = TCALC_VALTYPE_NUM;
      return tcalc_strtodouble(expr->token->val, &(out->as.num));
    }
    case TCALC_TOK_UNOP: {
      tcalc_val operand;
      tcalc_unopdef* unary_op_def;
      ret_on_err(err, tcalc_ctx_getunop(ctx, expr->token->val, &unary_op_def));
      ret_on_err(err, tcalc_eval_exprtree(expr->children[0], ctx, &operand));
      out->type = TCALC_VALTYPE_NUM;
      return unary_op_def->func(operand, &(out->as.num));
    }
    case TCALC_TOK_BINOP: {
      tcalc_val operand1, operand2;
      tcalc_binopdef* binary_op_def;
      ret_on_err(err, tcalc_ctx_getbinop(ctx, expr->token->val, &binary_op_def));
      ret_on_err(err, tcalc_eval_exprtree(expr->children[0], ctx, &operand1));
      ret_on_err(err, tcalc_eval_exprtree(expr->children[1], ctx, &operand2));
      out->type = TCALC_VALTYPE_NUM;
      return binary_op_def->func(operand1, operand2, &(out->as.num));
    }
    case TCALC_TOK_ID: {

      if (tcalc_ctx_hasunfunc(ctx, expr->token->val)) {
        tcalc_unfuncdef* unary_func_def;
        tcalc_ctx_getunfunc(ctx, expr->token->val, &unary_func_def);

        tcalc_val operand;
        ret_on_err(err, tcalc_eval_exprtree(expr->children[0], ctx, &operand));

        out->type = TCALC_VALTYPE_NUM;
        return unary_func_def->func(operand, &(out->as.num));
      } else if (tcalc_ctx_hasbinfunc(ctx, expr->token->val)) {
        tcalc_binfuncdef* binary_func_def;
        tcalc_ctx_getbinfunc(ctx, expr->token->val, &binary_func_def);

        tcalc_val operand1, operand2;
        ret_on_err(err, tcalc_eval_exprtree(expr->children[0], ctx, &operand1));
        ret_on_err(err, tcalc_eval_exprtree(expr->children[1], ctx, &operand2));

        out->type = TCALC_VALTYPE_NUM;
        return binary_func_def->func(operand1, operand2, &(out->as.num));
      } else if (tcalc_ctx_hasvar(ctx, expr->token->val)) {
        tcalc_vardef* vardef;
        tcalc_ctx_getvar(ctx, expr->token->val, &vardef);

        *out = TCALC_VAL_INIT_NUM(vardef->val);
        return TCALC_ERR_OK;
      } else {
        return TCALC_ERR_UNKNOWN_ID;
      }

    }
    default: {
      return TCALC_ERR_INVALID_ARG;
    }
  }
}

void tcalc_exprtree_free(tcalc_exprtree* head) {
  if (head == NULL) return;
  TCALC_ARR_FREE_CF(head->children, head->nb_children, tcalc_exprtree_free);
  tcalc_token_free(head->token);
  free(head);
}

void tcalc_exprtree_free_children(tcalc_exprtree* head) {
  if (head == NULL) return;
  for (size_t i = 0; i < head->nb_children; i++) {
    tcalc_exprtree_free(head->children[i]);
    head->children[i] = NULL;
  }
}


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

tcalc_err tcalc_exprtree_node_alloc(tcalc_token* token, size_t nb_children, tcalc_exprtree** out) {
  tcalc_err err;

  tcalc_exprtree* node = (tcalc_exprtree*)malloc(sizeof(tcalc_exprtree));
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
