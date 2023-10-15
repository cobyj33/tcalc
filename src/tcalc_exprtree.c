#include "tcalc_exprtree.h"

#include "tcalc_string.h"
#include "tcalc_func.h"
#include "tcalc_darray.h"
#include "tcalc_mem.h"

#include "string.h"
#include <stdlib.h>

tcalc_error_t tcalc_rpn_tokens_to_exprtree(tcalc_token_t** tokens, size_t nb_tokens, tcalc_exprtree_t** out);
void tcalc_exprtree_free_shallow(tcalc_exprtree_t* node);
tcalc_error_t tcalc_exprtree_alloc(tcalc_token_t* token, size_t nb_children, tcalc_exprtree_t** out);

/**
 * Pipeline:
 * 
 * Converts infix expression into infix tokens
 * 
 * Converts infix tokens into rpn-formatted tokens
 * 
 * Converts rpn-formatted tokens into returned expression tree
*/
tcalc_error_t tcalc_create_exprtree_infix(const char* infix, tcalc_exprtree_t** out) {
  tcalc_token_t** infix_tokens;
  size_t nb_infix_tokens;
  tcalc_error_t err = tcalc_tokenize_infix(infix, &infix_tokens, &nb_infix_tokens);
  if (err) return err;

  tcalc_token_t** rpn_tokens;
  size_t nb_rpn_tokens;
  err = tcalc_infix_tokens_to_rpn_tokens(infix_tokens, nb_infix_tokens, &rpn_tokens, &nb_rpn_tokens);
  tcalc_free_arr((void**)infix_tokens, nb_infix_tokens, tcalc_token_free);
  if (err) return err;

  err = tcalc_rpn_tokens_to_exprtree(rpn_tokens, nb_rpn_tokens, out);
  tcalc_free_arr((void**)rpn_tokens, nb_rpn_tokens, tcalc_token_free);

  return err;
}

tcalc_error_t tcalc_create_exprtree_rpn(const char* rpn, tcalc_exprtree_t** out) {
  tcalc_token_t** tokens;
  size_t nb_tokens;
  tcalc_error_t err = tcalc_tokenize_rpn(rpn, &tokens, &nb_tokens);
  if (err) return err;
  err = tcalc_rpn_tokens_to_exprtree(tokens, nb_tokens, out);
  tcalc_free_arr((void**)tokens, nb_tokens, tcalc_token_free);

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

  #define SUPPORTED_BINARY_OPERATIONS 5
  tcalc_exprtree_t** tree_stack = (tcalc_exprtree_t*)malloc(sizeof(tcalc_exprtree_t*) * nb_tokens);
  if (tree_stack == NULL) return TCALC_BAD_ALLOC;
  size_t tree_stack_size = 0; 

  for (size_t i = 0; i < nb_tokens; i++) {

    switch (tokens[i]->type) {
      case TCALC_NUMBER: {
        tcalc_exprtree_t* tree_node;
        if ((err = tcalc_exprtree_alloc(tokens[i], 0, &tree_node)) != TCALC_OK) {
          tcalc_free_arr((void**)tree_stack, tree_stack_size, tcalc_exprtree_free_shallow);
          return err;
        }

        tree_stack[tree_stack_size++] = tree_node;
        break;
      }
      case TCALC_BINARY_OPERATOR: {
        if (tree_stack_size < 2) {
          tcalc_free_arr((void**)tree_stack, tree_stack_size, tcalc_exprtree_free_shallow);
          return TCALC_INVALID_OP;
        }

        tcalc_exprtree_t* tree_node;
        if ((err = tcalc_exprtree_alloc(tokens[i], 2, &tree_node)) != TCALC_OK) {
          tcalc_free_arr((void**)tree_stack, tree_stack_size, tcalc_exprtree_free_shallow);
          return err;
        }

        tree_node->children[0] = tree_stack[tree_stack_size - 2];
        tree_node->children[1] = tree_stack[tree_stack_size - 1];

        tree_stack[tree_stack_size - 2] = tree_node;
        tree_stack_size--;
        break;
      }
      case TCALC_UNARY_OPERATOR: {
        if (tree_stack_size < 1) {
          free(tree_stack); // it's empty anyway
          return TCALC_INVALID_OP;
        }

        tcalc_exprtree_t* tree_node;
        if ((err = tcalc_exprtree_alloc(tokens[i], 1, &tree_node)) != TCALC_OK) {
          tcalc_free_arr((void**)tree_stack, tree_stack_size, tcalc_exprtree_free_shallow);
          return err;
        }

        tree_node->children[0] = tree_stack[tree_stack_size - 1];
        tree_stack[tree_stack_size - 1] = tree_node;
        break;
      }
      default: {
        tcalc_free_arr((void**)tree_stack, tree_stack_size, tcalc_exprtree_free_shallow);
        return TCALC_INVALID_OP;
      }
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

  #undef SUPPORTED_BINARY_OPERATIONS 
}

tcalc_error_t tcalc_exprtree_alloc(tcalc_token_t* token, size_t nb_children, tcalc_exprtree_t** out) {
  tcalc_error_t err;
  
  tcalc_exprtree_t* node = (tcalc_exprtree_t*)malloc(sizeof(tcalc_exprtree_t));
  if (node == NULL) return TCALC_BAD_ALLOC;
  
  if ((err = tcalc_token_copy(token, &node->token)) != TCALC_OK) {
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

typedef struct {
  tcalc_token_t token;
  int priority;
  tcalc_associativity_t associativity;
} tcalc_op_precedence_t;

tcalc_error_t tcalc_index_of_op_prec_data(const tcalc_op_precedence_t* operations, size_t nb_operations, tcalc_token_t* token, tcalc_op_precedence_t* out) {
  for (int i = 0; i < nb_operations; i++) {
    if (token->type == operations[i].token.type && strcmp(token->value, operations[i].token.value) == 0) {
      *out = operations[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

/**
 * 
 * Remember that the number of infix tokens and the number of rpn tokens are not
 * necessarily the same, as rpn doesn't have grouping tokens at all
 * 
 * This isn't in tcalc_tokens because it's really just an implementation step for 
 * creating an expression tree
*/
tcalc_error_t tcalc_infix_tokens_to_rpn_tokens(tcalc_token_t** infix_tokens, size_t nb_infix_tokens, tcalc_token_t*** out, size_t* out_size) {
  tcalc_error_t err;
  #define OP_PRECEDENCE_DEF_COUNT 7

  const tcalc_op_precedence_t OP_PRECEDENCE_DEFS[OP_PRECEDENCE_DEF_COUNT] = {
    {{TCALC_BINARY_OPERATOR, "+"}, 1, TCALC_LEFT_ASSOCIATIVE},
    {{TCALC_BINARY_OPERATOR, "-"}, 1, TCALC_LEFT_ASSOCIATIVE},
    {{TCALC_BINARY_OPERATOR, "*"}, 2, TCALC_LEFT_ASSOCIATIVE},
    {{TCALC_BINARY_OPERATOR, "/"}, 2, TCALC_LEFT_ASSOCIATIVE},
    {{TCALC_BINARY_OPERATOR, "^"}, 3, TCALC_RIGHT_ASSOCIATIVE},
    {{TCALC_UNARY_OPERATOR, "+"}, 4, TCALC_RIGHT_ASSOCIATIVE},
    {{TCALC_UNARY_OPERATOR, "-"}, 4, TCALC_RIGHT_ASSOCIATIVE},
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

  #define CLEAN_RETURN(predicate, tcalc_error) if ((predicate)) { \
                                  free(operator_stack); \
                                  tcalc_free_arr(rpn_tokens, rpn_tokens_size, tcalc_token_free); \
                                  *out_size = 0; \
                                  return (tcalc_error); }

  #define CLEAN_ERROR(tcalc_error_t_func_call) CLEAN_RETURN((err = (tcalc_error_t_func_call)) != TCALC_OK, err)
  // macros allow us to easily call errneous code without so many if blocks

  for (size_t i = 0; i < nb_infix_tokens; i++) {
    switch (infix_tokens[i]->type) {
      case TCALC_NUMBER: {
        tcalc_token_t* copy;
        CLEAN_ERROR(tcalc_token_copy(infix_tokens[i], &rpn_tokens[rpn_tokens_size]))
        rpn_tokens_size++;
        break;
      }
      case TCALC_GROUP_START: { // "("
        operator_stack[operator_stack_size++] = infix_tokens[i];
        break;
      }
      case TCALC_GROUP_END: { // ")"
        CLEAN_RETURN(operator_stack_size == 0, TCALC_INVALID_OP) // ending parenthesis found with no opening parenthesis

        while (strcmp(operator_stack[operator_stack_size - 1]->value, "(") != 0) { // keep popping onto output until the opening parenthesis is found
          CLEAN_ERROR(tcalc_token_copy(operator_stack[operator_stack_size - 1], &rpn_tokens[rpn_tokens_size]))
          rpn_tokens_size++;
          operator_stack_size--;
          CLEAN_RETURN(operator_stack_size == 0, TCALC_INVALID_OP) // no opening parenthesis was found
        }
        operator_stack_size--; // pop off opening parenthesis

        break;
      }
      case TCALC_UNARY_OPERATOR:
      case TCALC_BINARY_OPERATOR: {
        tcalc_op_precedence_t current_optdef, stack_optdef;
        CLEAN_ERROR(tcalc_index_of_op_prec_data(OP_PRECEDENCE_DEFS, OP_PRECEDENCE_DEF_COUNT, infix_tokens[i], &current_optdef))

        if (operator_stack_size > 0) {
          while (operator_stack_size > 0 && tcalc_index_of_op_prec_data(OP_PRECEDENCE_DEFS, OP_PRECEDENCE_DEF_COUNT, operator_stack[operator_stack_size - 1], &stack_optdef) == TCALC_OK) {
            if (stack_optdef.priority > current_optdef.priority || (stack_optdef.priority == current_optdef.priority && current_optdef.associativity == TCALC_LEFT_ASSOCIATIVE)) {
              CLEAN_ERROR(tcalc_token_copy(operator_stack[operator_stack_size - 1], &rpn_tokens[rpn_tokens_size]))
              rpn_tokens_size++;
              operator_stack_size--;
            } else { break; }
          }
        }

        operator_stack[operator_stack_size++] = infix_tokens[i];
        break;
      }
      default: {
        CLEAN_RETURN(1, TCALC_UNIMPLEMENTED);
      }
    }
  }

  while (operator_stack_size > 0) {
    CLEAN_ERROR(tcalc_token_copy(operator_stack[operator_stack_size - 1], &rpn_tokens[rpn_tokens_size]))
    rpn_tokens_size++;
    operator_stack_size--;
  }

  free(operator_stack);

  *out = rpn_tokens;
  *out_size = rpn_tokens_size;
  return TCALC_OK;

  #undef CLEAN_RETURN
  #undef CLEAN_ERROR
  #undef OP_PRECEDENCE_DEF_COUNT
}

