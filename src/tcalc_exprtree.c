#include "tcalc_exprtree.h"

#include "tcalc_string.h"
#include "tcalc_func.h"
#include "tcalc_mem.h"

#include "string.h"
#include <stdlib.h>

tcalc_error_t tcalc_rpn_tokens_to_exprtree(tcalc_token_t** tokens, size_t nb_tokens, tcalc_exprtree_t** out);
void tcalc_exprtree_free_shallow(tcalc_exprtree_t* node);
void tcalc_exprtree_freev_shallow(void* node);
tcalc_error_t tcalc_exprtree_alloc(tcalc_token_t* token, size_t nb_children, tcalc_exprtree_t** out);

/**
 * Pipeline:
 * 
 * Convert infix expression into infix tokens
 * Convert infix tokens into rpn-formatted tokens
 * Convert rpn-formatted tokens into returned expression tree
*/
tcalc_error_t tcalc_create_exprtree_infix(const char* infix, tcalc_exprtree_t** out) {
  tcalc_token_t** infix_tokens;
  size_t nb_infix_tokens;
  tcalc_error_t err = tcalc_tokenize_infix(infix, &infix_tokens, &nb_infix_tokens);
  if (err) return err;

  tcalc_token_t** rpn_tokens;
  size_t nb_rpn_tokens;
  err = tcalc_infix_tokens_to_rpn_tokens(infix_tokens, nb_infix_tokens, &rpn_tokens, &nb_rpn_tokens);
  tcalc_free_arr((void**)infix_tokens, nb_infix_tokens, tcalc_token_freev);
  if (err) return err;

  err = tcalc_rpn_tokens_to_exprtree(rpn_tokens, nb_rpn_tokens, out);
  tcalc_free_arr((void**)rpn_tokens, nb_rpn_tokens, tcalc_token_freev);

  return err;
}

tcalc_error_t tcalc_create_exprtree_rpn(const char* rpn, tcalc_exprtree_t** out) {
  tcalc_token_t** tokens;
  size_t nb_tokens;
  tcalc_error_t err = tcalc_tokenize_rpn(rpn, &tokens, &nb_tokens);
  if (err) return err;
  err = tcalc_rpn_tokens_to_exprtree(tokens, nb_tokens, out);
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
tcalc_error_t tcalc_eval_exprtree(tcalc_exprtree_t* expr, double* out) {
  switch (expr->token->type) {
    case TCALC_NUMBER: { 
      tcalc_error_t err = tcalc_strtodouble(expr->token->value, out);
      if (err) return err;
      return TCALC_OK;
    }
    case TCALC_UNARY_OPERATOR: {
      double operand;
      tcalc_error_t err = tcalc_eval_exprtree(expr->children[0], &operand);
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
      tcalc_error_t err = tcalc_eval_exprtree(expr->children[0], &operand1);
      if (err) return err;
      err = tcalc_eval_exprtree(expr->children[1], &operand2);
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
      } else {
        return TCALC_INVALID_ARG;
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

tcalc_error_t tcalc_rpn_tokens_to_exprtree(tcalc_token_t** tokens, size_t nb_tokens, tcalc_exprtree_t** out) {
  tcalc_error_t err = TCALC_OK;

  tcalc_exprtree_t** tree_stack = (tcalc_exprtree_t**)malloc(sizeof(tcalc_exprtree_t*) * nb_tokens);
  if (tree_stack == NULL) return TCALC_BAD_ALLOC;
  size_t tree_stack_size = 0; 

  for (size_t i = 0; i < nb_tokens; i++) {

    switch (tokens[i]->type) {
      case TCALC_NUMBER: {
        tcalc_exprtree_t* tree_node;
        if ((err = tcalc_exprtree_alloc(tokens[i], 0, &tree_node)) != TCALC_OK)
          goto cleanup;

        tree_stack[tree_stack_size++] = tree_node;
        break;
      }
      case TCALC_BINARY_OPERATOR: {
        if (tree_stack_size < 2) {
          err = TCALC_INVALID_OP;
          goto cleanup;
        }

        tcalc_exprtree_t* tree_node;
        if ((err = tcalc_exprtree_alloc(tokens[i], 2, &tree_node)) != TCALC_OK)
          goto cleanup;

        tree_node->children[0] = tree_stack[tree_stack_size - 2];
        tree_node->children[1] = tree_stack[tree_stack_size - 1];

        tree_stack[tree_stack_size - 2] = tree_node;
        tree_stack_size--;
        break;
      }
      case TCALC_UNARY_OPERATOR: {
        if (tree_stack_size < 1) {
          err = TCALC_INVALID_OP;
          goto cleanup; 
        }

        tcalc_exprtree_t* tree_node;
        if ((err = tcalc_exprtree_alloc(tokens[i], 1, &tree_node)) != TCALC_OK)
          goto cleanup;

        tree_node->children[0] = tree_stack[tree_stack_size - 1];
        tree_stack[tree_stack_size - 1] = tree_node;
        break;
      }
      default: goto cleanup;
    }
  }

  err = TCALC_OK;
  if (tree_stack_size == 1) {
    *out = tree_stack[0];
  } else {
    err = TCALC_INVALID_ARG;
  }

  free(tree_stack);
  return err;

  cleanup:
    tcalc_free_arr((void**)tree_stack, tree_stack_size, tcalc_exprtree_freev_shallow);
    return err;
}





tcalc_error_t tcalc_exprtree_alloc(tcalc_token_t* token, size_t nb_children, tcalc_exprtree_t** out) {
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

void tcalc_exprtree_free_shallow(tcalc_exprtree_t* node) {
  tcalc_token_free(node->token);
  free(node->children);
  free(node);
}

void tcalc_exprtree_freev_shallow(void* node) {
  tcalc_exprtree_free_shallow((tcalc_exprtree_t*)node);
}
