#include "tcalc_exprtree.h"

#include "tcalc_string.h"
#include "tcalc_func.h"
#include "tcalc_mem.h"

#include "string.h"
#include <stdlib.h>

tcalc_error_t tcalc_rpn_tokens_to_exprtree(tcalc_token_t** tokens, size_t nb_tokens, const tcalc_context_t* context, tcalc_exprtree_t** out);

/**
 * Pipeline:
 * 
 * Convert infix expression into infix tokens
 * Convert infix tokens into rpn-formatted tokens
 * Convert rpn-formatted tokens into returned expression tree
*/
tcalc_error_t tcalc_create_exprtree_infix(const char* infix, const tcalc_context_t* context, tcalc_exprtree_t** out) {
  tcalc_token_t** infix_tokens;
  size_t nb_infix_tokens;
  tcalc_error_t err = tcalc_tokenize_infix(infix, &infix_tokens, &nb_infix_tokens);
  if (err) return err;

  tcalc_token_t** rpn_tokens;
  size_t nb_rpn_tokens;
  err = tcalc_infix_tokens_to_rpn_tokens(infix_tokens, nb_infix_tokens, context, &rpn_tokens, &nb_rpn_tokens);
  tcalc_free_arr((void**)infix_tokens, nb_infix_tokens, tcalc_token_freev);
  if (err) return err;

  err = tcalc_rpn_tokens_to_exprtree(rpn_tokens, nb_rpn_tokens, context, out);
  tcalc_free_arr((void**)rpn_tokens, nb_rpn_tokens, tcalc_token_freev);
  return err;
}

tcalc_error_t tcalc_create_exprtree_rpn(const char* rpn, const tcalc_context_t* context, tcalc_exprtree_t** out) {
  tcalc_token_t** tokens;
  size_t nb_tokens;
  tcalc_error_t err = tcalc_tokenize_rpn(rpn, &tokens, &nb_tokens);
  if (err) return err;

  err = tcalc_rpn_tokens_to_exprtree(tokens, nb_tokens, context, out);
  tcalc_free_arr((void**)tokens, nb_tokens, tcalc_token_freev);
  return err;
}


/**
 * Detailed explanation of how an expression tree is evaluated
 * 
 * If the token type is a number:
 *  - simply read the number value in the 
 * 
 * If the token type is a unary operator
 * 
*/
tcalc_error_t tcalc_eval_exprtree(tcalc_exprtree_t* expr, const tcalc_context_t* context, double* out) {
  switch (expr->token->type) {
    case TCALC_NUMBER: { 
      return tcalc_strtodouble(expr->token->value, out);
    }
    case TCALC_UNARY_OPERATOR: {
      double operand;
      tcalc_error_t err = tcalc_eval_exprtree(expr->children[0], context, &operand);
      if (err) return err;

      if (strcmp(expr->token->value, "+") == 0) {
        return tcalc_unary_plus(operand, out);
      } else if (strcmp(expr->token->value, "-") == 0) {
        return tcalc_unary_minus(operand, out);
      } else {
        return TCALC_INVALID_ARG;
      } 
    }
    case TCALC_BINARY_OPERATOR: {
      double operand1;
      double operand2;
      tcalc_error_t err = tcalc_eval_exprtree(expr->children[0], context, &operand1);
      if (err) return err;
      err = tcalc_eval_exprtree(expr->children[1], context, &operand2);
      if (err) return err;
      
      if (strcmp(expr->token->value, "+") == 0) {
        return tcalc_add(operand1, operand2, out);
      } else if (strcmp(expr->token->value, "-") == 0) {
        return tcalc_subtract(operand1, operand2, out);
      } else if (strcmp(expr->token->value, "*") == 0) {
        return tcalc_multiply(operand1, operand2, out);
      } else if (strcmp(expr->token->value, "/") == 0) {
        return tcalc_divide(operand1, operand2, out);
      } else if (strcmp(expr->token->value, "^") == 0) {
        return tcalc_pow(operand1, operand2, out);
      } else if (strcmp(expr->token->value, "%") == 0) {
        return tcalc_mod(operand1, operand2, out);
      } else {
        return TCALC_INVALID_ARG;
      }
    }
    case TCALC_IDENTIFIER: {

      if (tcalc_context_has_unary_func(context, expr->token->value) == TCALC_OK) {
        tcalc_unary_func_def_t unary_func_def;
        tcalc_context_get_unary_func(context, expr->token->value, &unary_func_def);
        
        double operand;
        tcalc_error_t err = tcalc_eval_exprtree(expr->children[0], context, &operand);
        if (err) return err;

        return unary_func_def.function(operand, out);
      } else if (tcalc_context_has_binary_func(context, expr->token->value) == TCALC_OK) {
        tcalc_binary_func_def_t binary_func_def;
        tcalc_context_get_binary_func(context, expr->token->value, &binary_func_def);

        double operand1;
        double operand2;
        tcalc_error_t err = tcalc_eval_exprtree(expr->children[0], context, &operand1);
        if (err) return err;
        err = tcalc_eval_exprtree(expr->children[1], context, &operand2);
        if (err) return err;
      
        return binary_func_def.function(operand1, operand2, out);
      } else if (tcalc_context_has_variable(context, expr->token->value) == TCALC_OK) {
        tcalc_variable_def_t vardef;
        tcalc_context_get_variable(context, expr->token->value, &vardef);

        *out = vardef.value;
        return TCALC_OK;
      } else {
        return TCALC_UNKNOWN_IDENTIFIER;
      }
      
    }
    default: {
      return TCALC_INVALID_ARG;
    }
  }
}

void tcalc_exprtree_free(tcalc_exprtree_t* head) {
  for (size_t i = 0; i < head->nb_children; i++) {
    tcalc_exprtree_free(head->children[i]);
  }

  tcalc_token_free(head->token);
  free(head);
}

/**
 * Fun fact, the rpn representation of a 
 * 
 * 
*/
tcalc_error_t tcalc_rpn_tokens_to_exprtree(tcalc_token_t** tokens, size_t nb_tokens, const tcalc_context_t* context, tcalc_exprtree_t** out) {
  tcalc_error_t err = TCALC_UNKNOWN;

  tcalc_exprtree_t** tree_stack = (tcalc_exprtree_t**)malloc(sizeof(tcalc_exprtree_t*) * nb_tokens);
  if (tree_stack == NULL) return TCALC_BAD_ALLOC;
  size_t tree_stack_size = 0; 

  for (size_t i = 0; i < nb_tokens; i++) {

    switch (tokens[i]->type) {
      case TCALC_NUMBER: {
        tcalc_exprtree_t* tree_node;
        if ((err = tcalc_exprtree_node_alloc(tokens[i], 0, &tree_node)) != TCALC_OK) goto cleanup;
        tree_stack[tree_stack_size++] = tree_node;
        break;
      } // TCALC_NUMBER
      case TCALC_BINARY_OPERATOR: {
        if (tree_stack_size < 2) {
          err = TCALC_INVALID_OP;
          goto cleanup;
        }

        tcalc_exprtree_t* tree_node;
        if ((err = tcalc_exprtree_node_alloc(tokens[i], 2, &tree_node)) != TCALC_OK) goto cleanup;

        tree_node->children[0] = tree_stack[tree_stack_size - 2];
        tree_node->children[1] = tree_stack[tree_stack_size - 1];

        tree_stack[tree_stack_size - 2] = tree_node;
        tree_stack_size--;
        break;
      } // TCALC_BINARY_OPERATOR
      case TCALC_UNARY_OPERATOR: {
        if (tree_stack_size < 1) {
          err = TCALC_INVALID_OP;
          goto cleanup; 
        }

        tcalc_exprtree_t* tree_node;
        if ((err = tcalc_exprtree_node_alloc(tokens[i], 1, &tree_node)) != TCALC_OK) goto cleanup;

        tree_node->children[0] = tree_stack[tree_stack_size - 1];
        tree_stack[tree_stack_size - 1] = tree_node;
        break;
      } // TCALC_UNARY_OPERATOR
      case TCALC_IDENTIFIER: {

        if (tcalc_context_has_variable(context, tokens[i]->value) == TCALC_OK) {

          tcalc_exprtree_t* tree_node;
          if ((err = tcalc_exprtree_node_alloc(tokens[i], 0, &tree_node)) != TCALC_OK) goto cleanup;
          tree_stack[tree_stack_size++] = tree_node;

        } else if (tcalc_context_has_binary_func(context, tokens[i]->value) == TCALC_OK) {
          if (tree_stack_size < 2) {
            err = TCALC_INVALID_OP;
            goto cleanup; 
          }

          tcalc_exprtree_t* tree_node;
          if ((err = tcalc_exprtree_node_alloc(tokens[i], 2, &tree_node)) != TCALC_OK) goto cleanup;

          tree_node->children[0] = tree_stack[tree_stack_size - 2];
          tree_node->children[1] = tree_stack[tree_stack_size - 1];

          tree_stack[tree_stack_size - 2] = tree_node;
          tree_stack_size--;
        } else if (tcalc_context_has_unary_func(context, tokens[i]->value) == TCALC_OK) {
          if (tree_stack_size < 1) {
            err = TCALC_INVALID_OP;
            goto cleanup; 
          }

          tcalc_exprtree_t* tree_node;
          if ((err = tcalc_exprtree_node_alloc(tokens[i], 1, &tree_node)) != TCALC_OK) goto cleanup;

          tree_node->children[0] = tree_stack[tree_stack_size - 1];
          tree_stack[tree_stack_size - 1] = tree_node;
        } else {
          err = TCALC_UNKNOWN_IDENTIFIER;
          goto cleanup;
        }

        break;
      } // TCALC_IDENTIFIER
      default: goto cleanup;
    }
  }

  err = TCALC_OK;
  if (tree_stack_size == 1) {
    *out = tree_stack[0];
  } else {
    err = TCALC_INVALID_ARG;
    goto cleanup;
  }

  free(tree_stack);
  return err;

  cleanup:
    tcalc_free_arr((void**)tree_stack, tree_stack_size, tcalc_exprtree_node_freev);
    return err;
}

tcalc_error_t tcalc_exprtree_node_alloc(tcalc_token_t* token, size_t nb_children, tcalc_exprtree_t** out) {
  tcalc_error_t err;
  
  tcalc_exprtree_t* node = (tcalc_exprtree_t*)malloc(sizeof(tcalc_exprtree_t));
  if (node == NULL) return TCALC_BAD_ALLOC;
  
  if ((err = tcalc_token_clone(token, &node->token)) != TCALC_OK) {
    free(node);
    return err;
  }

  node->children = NULL;
  node->nb_children = nb_children;

  if (nb_children > 0) {
    node->children = (tcalc_exprtree_t**)malloc(sizeof(tcalc_exprtree_t*) * nb_children);
    if (node->children == NULL) {
      tcalc_token_free(node->token);
      free(node);
      return TCALC_BAD_ALLOC;
    }
  }

  *out = node;
  return TCALC_OK;
}

void tcalc_exprtree_node_free(tcalc_exprtree_t* node) {
  tcalc_token_free(node->token);
  free(node->children);
  free(node);
}

void tcalc_exprtree_node_freev(void* node) {
  tcalc_exprtree_node_free((tcalc_exprtree_t*)node);
}
