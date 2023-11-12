#ifndef TCALC_CONTEXT_H
#define TCALC_CONTEXT_H

#include "tcalc_func_type.h"
#include "tcalc_tokens.h"
#include "tcalc_error.h"

#include <stddef.h>

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
  const char* identifier;
  double value;
} tcalc_variable_def_t;

typedef struct tcalc_variable_def_expr_t {
  char* name;
  tcalc_exprtree_t* expr;
} tcalc_variable_def_expr_t;

typedef enum tcalc_associativity_t{
  TCALC_RIGHT_ASSOCIATIVE,
  TCALC_LEFT_ASSOCIATIVE,
} tcalc_associativity_t;

typedef struct tcalc_precedence_t {
  int priority;
  tcalc_associativity_t associativity;
} tcalc_precedence_t;

typedef struct tcalc_unary_op_def_t {
  char symbol;
  tcalc_precedence_t precedence;
  tcalc_unary_func function;
} tcalc_unary_op_def_t;

typedef struct tcalc_binary_op_def_t {
  char symbol;
  tcalc_precedence_t precedence;
  tcalc_binary_func function;
} tcalc_binary_op_def_t;

typedef struct tcalc_unary_func_def_t {
  const char* identifier;
  tcalc_unary_func function;
} tcalc_unary_func_def_t;

typedef struct tcalc_binary_func_def_t {
  const char* identifier;
  tcalc_binary_func function;
} tcalc_binary_func_def_t;

typedef struct tcalc_grouping_symbol_def_t {
  char start_symbol;
  char end_symbol;
} tcalc_grouping_symbol_def_t;

// these can be tweaked as they're needed. I just gave them values
// based on general math symbols.
#define TCALC_CONTEXT_MAX_UNARY_OP_DEFS 4
#define TCALC_CONTEXT_MAX_BINARY_OP_DEFS 8
#define TCALC_CONTEXT_MAX_GROUPING_SYMBOL_DEFS 6
#define TCALC_CONTEXT_MAX_UNARY_FUNC_DEFS 64
#define TCALC_CONTEXT_MAX_BINARY_FUNC_DEFS 32
#define TCALC_CONTEXT_MAX_VARIABLE_DEFS 16

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
  tcalc_unary_func_def_t unary_funcs[TCALC_CONTEXT_MAX_UNARY_FUNC_DEFS];
  size_t nb_unary_funcs;
  tcalc_binary_func_def_t binary_funcs[TCALC_CONTEXT_MAX_BINARY_FUNC_DEFS];
  size_t nb_binary_funcs;
  tcalc_variable_def_t variables[TCALC_CONTEXT_MAX_VARIABLE_DEFS];
  size_t nb_variables;
} tcalc_context_t;

extern const tcalc_context_t TCALC_GLOBAL_CONTEXT;

int tcalc_context_has_identifier(const tcalc_context_t* context, const char* name);
int tcalc_context_has_func(const tcalc_context_t* context, const char* name);
int tcalc_context_has_unary_func(const tcalc_context_t* context, const char* name);
int tcalc_context_has_binary_func(const tcalc_context_t* context, const char* name);
int tcalc_context_has_variable(const tcalc_context_t* context, const char* name);

/**
 * tcalc_context_get_x functions will not return an error whenever the same given
 * context and name parameters return truthy from their corresponding tcalc_context_has_x
 * functions 
*/

tcalc_error_t tcalc_context_get_unary_func(const tcalc_context_t* context, const char* name, tcalc_unary_func_def_t* out);
tcalc_error_t tcalc_context_get_binary_func(const tcalc_context_t* context, const char* name, tcalc_binary_func_def_t* out);
tcalc_error_t tcalc_context_get_variable(const tcalc_context_t* context, const char* name, tcalc_variable_def_t* out);





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