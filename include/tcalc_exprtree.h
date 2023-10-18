#ifndef TCALC_PARSETREE_H
#define TCALC_PARSETREE_H

#include <stddef.h>
#include "tcalc_tokens.h"
#include "tcalc_error.h"
#include "tcalc_context.h"

/**
 * 
 * The type of token dictates how an expression tree node should be evaluated,
 * as well as how many
 * 
 * TCALC_NUMBER:
 *  The token simply holds a number in its value string, and it can be evaluated
 *  by calling tcalc_strtodouble. The expression tree node has no children.
 * 
 * TCALC_UNARY_OPERATOR:
 *  The expression tree node has 1 child which must be evaluated, and then the
 *  corresponding operator in the token's value is used to determine the unary
 *  operation to perform on the evaluated child.
 * 
 * TCALC_BINARY_OPERATOR:
 *  The expression tree node has 2 children which must first be evaluated, and then the
 *  corresponding operator in the token's value is used to determine the binary
 *  operation to perform on the evaluated children. The evaluated children must
 *  be called with the binary expression in order, as some binary operations like
 *  division and subtraction are not associative.
 * 
 * Every other token type is currently unimplemented :(
 * 
 * To validate a tree, evaluate it with tcalc_eval_exprtree and checking that
 * it returns TCALC_OK. 
*/
typedef struct tcalc_exprtree_node_t {
  tcalc_token_t* token;
  struct tcalc_exprtree_node_t** children;
  size_t nb_children;
} tcalc_exprtree_t;

tcalc_error_t tcalc_exprtree_node_alloc(tcalc_token_t* token, size_t nb_children, tcalc_exprtree_t** out);
void tcalc_exprtree_node_free(tcalc_exprtree_t* node);
void tcalc_exprtree_node_freev(void* node);
/**
 * Free a tcalc expression tree recursively
*/
void tcalc_exprtree_free(tcalc_exprtree_t* head);

tcalc_error_t tcalc_create_exprtree_rpn(const char* rpn, const tcalc_context_t* context, tcalc_exprtree_t** out);
tcalc_error_t tcalc_create_exprtree_infix(const char* infix, const tcalc_context_t* context, tcalc_exprtree_t** out);
tcalc_error_t tcalc_infix_tokens_to_rpn_tokens(tcalc_token_t** tokens, size_t nb_tokens, const tcalc_context_t* context, tcalc_token_t*** out, size_t* out_size);



tcalc_error_t tcalc_eval_exprtree(tcalc_exprtree_t* expr, const tcalc_context_t* context, double* out);



#endif