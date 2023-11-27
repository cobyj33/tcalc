#ifndef TCALC_CONTEXT_H
#define TCALC_CONTEXT_H

#include "tcalc_tokens.h"
#include "tcalc_error.h"
#include "tcalc_func.h"

#include <stddef.h>

typedef enum tcalc_associativity_t{
  TCALC_RIGHT_ASSOCIATIVE,
  TCALC_LEFT_ASSOCIATIVE,
} tcalc_associativity_t;

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

typedef struct tcalc_variable_def_t {
  char* identifier;
  double value;
} tcalc_variable_def_t;

typedef struct tcalc_op_data_t {
  int precedence;
  tcalc_associativity_t associativity;
} tcalc_op_data_t;

typedef struct tcalc_variable_def_expr_t {
  char* name;
  tcalc_exprtree_t* expr;
  char* dependencies;
} tcalc_variable_def_expr_t;


typedef struct tcalc_unary_op_def_t {
  char* identifier;
  int precedence;
  tcalc_associativity_t associativity;
  tcalc_unary_func function;
} tcalc_unary_op_def_t;

typedef struct tcalc_binary_op_def_t {
  char* identifier;
  int precedence;
  tcalc_associativity_t associativity;
  tcalc_binary_func function;
} tcalc_binary_op_def_t;

typedef struct tcalc_unary_func_def_t {
  char* identifier;
  tcalc_unary_func function;
} tcalc_unary_func_def_t;

typedef struct tcalc_binary_func_def_t {
  char* identifier;
  tcalc_binary_func function;
} tcalc_binary_func_def_t;

typedef struct tcalc_grouping_symbol_def_t {
  char start_symbol;
  char end_symbol;
} tcalc_grouping_symbol_def_t;

tcalc_op_data_t tcalc_binary_op_get_data(tcalc_binary_op_def_t* binary_op_def);
tcalc_op_data_t tcalc_unary_op_get_data(tcalc_unary_op_def_t* unary_op_def);

/**
 * Some conditions to a tcalc context:
 * 
 * Unary function identifiers must only contain alphabetical characters
 * Binary function identifiers must only contain alphabetical characters
 * Variable identifiers must only contain aphabetical characters
 * 
 * Symbols and identifiers must NOT be the same across the tcalc_context 
 * - The only exception to the above rule is that unary operators and binary operators
 *  can have the same symbol identifiers. This should be taken with salt though, as the tokenizer
 *  will disambiguate itself which + or - are unary and which + or - are binary. 
 *  If I had created math I wouldn't have created it like this.
*/
// typedef struct tcalc_context_t {
//   tcalc_unary_func_def_t unary_funcs[TCALC_CONTEXT_MAX_UNARY_FUNC_DEFS];
//   size_t nb_unary_funcs;
//   tcalc_binary_func_def_t binary_funcs[TCALC_CONTEXT_MAX_BINARY_FUNC_DEFS];
//   size_t nb_binary_funcs;
//   tcalc_variable_def_t variables[TCALC_CONTEXT_MAX_VARIABLE_DEFS];
//   size_t nb_variables;
// } tcalc_context_t;

typedef struct tcalc_context_t {
  tcalc_unary_func_def_t** unary_funcs;
  size_t nb_unary_funcs;
  size_t unary_funcs_capacity;

  tcalc_binary_func_def_t** binary_funcs;
  size_t nb_binary_funcs;
  size_t binary_funcs_capacity;

  tcalc_variable_def_t** variables;
  size_t nb_variables;
  size_t variables_capacity;

  tcalc_unary_op_def_t** unary_ops;
  size_t nb_unary_ops;
  size_t unary_ops_capacity;

  tcalc_binary_op_def_t** binary_ops;
  size_t nb_binary_ops;
  size_t binary_ops_capacity;
} tcalc_context_t;

tcalc_error_t tcalc_context_alloc_empty(tcalc_context_t** out);
tcalc_error_t tcalc_context_alloc_default(tcalc_context_t** out);

void tcalc_context_free(tcalc_context_t* context);

tcalc_error_t tcalc_context_add_variable(tcalc_context_t* context, char* name, double value);
tcalc_error_t tcalc_context_add_unary_func(tcalc_context_t* context, char* name, tcalc_unary_func function);
tcalc_error_t tcalc_context_add_binary_func(tcalc_context_t* context, char* name, tcalc_binary_func function);
tcalc_error_t tcalc_context_add_unary_op(tcalc_context_t* context, char* name, int precedence, tcalc_associativity_t associativity, tcalc_unary_func function);
tcalc_error_t tcalc_context_add_binary_op(tcalc_context_t* context,  char* name, int precedence, tcalc_associativity_t associativity, tcalc_binary_func function);

int tcalc_context_has_identifier(const tcalc_context_t* context, const char* name);
int tcalc_context_has_func(const tcalc_context_t* context, const char* name);

int tcalc_context_has_op(const tcalc_context_t* context, const char* name);
int tcalc_context_has_unary_op(const tcalc_context_t* context, const char* name);
int tcalc_context_has_binary_op(const tcalc_context_t* context, const char* name);
int tcalc_context_has_unary_func(const tcalc_context_t* context, const char* name);
int tcalc_context_has_binary_func(const tcalc_context_t* context, const char* name);
int tcalc_context_has_variable(const tcalc_context_t* context, const char* name);

/**
 * tcalc_context_get_x functions will not return an error whenever the same given
 * context and name parameters return truthy from their corresponding tcalc_context_has_x
 * functions 
*/

tcalc_error_t tcalc_context_get_unary_func(const tcalc_context_t* context, const char* name, tcalc_unary_func_def_t** out);
tcalc_error_t tcalc_context_get_binary_func(const tcalc_context_t* context, const char* name, tcalc_binary_func_def_t** out);
tcalc_error_t tcalc_context_get_variable(const tcalc_context_t* context, const char* name, tcalc_variable_def_t** out);
tcalc_error_t tcalc_context_get_unary_op(const tcalc_context_t* context, const char* name, tcalc_unary_op_def_t** out);
tcalc_error_t tcalc_context_get_binary_op(const tcalc_context_t* context, const char* name, tcalc_binary_op_def_t** out);

tcalc_error_t tcalc_context_get_op_data(const tcalc_context_t* context, const char* name, tcalc_op_data_t* out);

tcalc_error_t tcalc_exprtree_node_alloc(tcalc_token_t* token, size_t nb_children, tcalc_exprtree_t** out);
void tcalc_exprtree_node_free(tcalc_exprtree_t* node);
void tcalc_exprtree_node_freev(void* node);

/**
 * Free a tcalc expression tree recursively
*/
void tcalc_exprtree_free(tcalc_exprtree_t* head);

int tcalc_exprtree_is_vardef(tcalc_exprtree_t* expr);

tcalc_error_t tcalc_create_exprtree_rpn(const char* rpn, const tcalc_context_t* context, tcalc_exprtree_t** out);
tcalc_error_t tcalc_create_exprtree_infix(const char* infix, const tcalc_context_t* context, tcalc_exprtree_t** out);
tcalc_error_t tcalc_infix_tokens_to_rpn_tokens(tcalc_token_t** tokens, size_t nb_tokens, const tcalc_context_t* context, tcalc_token_t*** out, size_t* out_size);

tcalc_error_t tcalc_eval_exprtree(tcalc_exprtree_t* expr, const tcalc_context_t* context, double* out);

#endif