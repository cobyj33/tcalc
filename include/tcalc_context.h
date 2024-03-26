#ifndef TCALC_CONTEXT_H
#define TCALC_CONTEXT_H

#include "tcalc_tokens.h"
#include "tcalc_error.h"
#include "tcalc_func.h"
#include "tcalc_vec.h"

#include <stddef.h>

typedef enum tcalc_assoc{
  TCALC_RIGHT_ASSOCIATIVE,
  TCALC_LEFT_ASSOCIATIVE,
} tcalc_assoc;

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
 * 
 * Every other token type is currently unimplemented :(
 * 
 * To validate a tree, evaluate it with tcalc_eval_exprtree and checking that
 * it returns TCALC_OK. 
*/
typedef struct tcalc_exprtree_node {
  tcalc_token* token;
  struct tcalc_exprtree_node** children;
  size_t nb_children;
} tcalc_exprtree;

enum tcalc_exprtree_node_type {
  TCALC_EXPRTREE_TYPE_EXPRESSION,
  TCALC_EXPRTREE_TYPE_RELATION
};

typedef struct tcalc_vardef {
  char* identifier;
  double value;
} tcalc_vardef;

typedef struct tcalc_opdata {
  int precedence;
  tcalc_assoc associativity;
} tcalc_opdata;

typedef struct tcalc_exprvardef {
  char* name;
  tcalc_exprtree* expr;
  char* dependencies;
} tcalc_exprvardef;


typedef struct tcalc_unary_opdef {
  char* identifier;
  int precedence;
  tcalc_assoc associativity;
  tcalc_unary_func function;
} tcalc_unary_opdef;

typedef struct tcalc_relation_opdef {
  char* identifier;
  int precedence;
  tcalc_assoc associativity;
  tcalc_relation_func function;
} tcalc_relation_opdef;

typedef struct tcalc_binary_opdef {
  char* identifier;
  int precedence;
  tcalc_assoc associativity;
  tcalc_binary_func function;
} tcalc_binary_opdef;

typedef struct tcalc_unary_funcdef {
  char* identifier;
  tcalc_unary_func function;
} tcalc_unary_funcdef;

typedef struct tcalc_binary_funcdef {
  char* identifier;
  tcalc_binary_func function;
} tcalc_binary_funcdef;

typedef struct tcalc_groupsymdef {
  char start_symbol;
  char end_symbol;
} tcalc_groupsymdef;

tcalc_opdata tcalc_binary_op_get_data(tcalc_binary_opdef* binary_op_def);
tcalc_opdata tcalc_unary_op_get_data(tcalc_unary_opdef* unary_op_def);

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
typedef struct tcalc_context {
  TCALC_VEC(tcalc_unary_funcdef*) unary_funcs;
  TCALC_VEC(tcalc_binary_funcdef*) binary_funcs;
  TCALC_VEC(tcalc_vardef*) variables;
  TCALC_VEC(tcalc_unary_opdef*) unary_ops;
  TCALC_VEC(tcalc_binary_opdef*) binary_ops;
  TCALC_VEC(tcalc_relation_opdef*) relation_ops;
} tcalc_context;

tcalc_error_t tcalc_context_alloc_empty(tcalc_context** out);
tcalc_error_t tcalc_context_alloc_default(tcalc_context** out);

void tcalc_context_free(tcalc_context* context);

tcalc_error_t tcalc_context_add_variable(tcalc_context* context, char* name, double value);
tcalc_error_t tcalc_context_add_unary_func(tcalc_context* context, char* name, tcalc_unary_func function);
tcalc_error_t tcalc_context_add_binary_func(tcalc_context* context, char* name, tcalc_binary_func function);
tcalc_error_t tcalc_context_add_unary_op(tcalc_context* context, char* name, int precedence, tcalc_assoc associativity, tcalc_unary_func function);
tcalc_error_t tcalc_context_add_binary_op(tcalc_context* context,  char* name, int precedence, tcalc_assoc associativity, tcalc_binary_func function);

int tcalc_context_has_identifier(const tcalc_context* context, const char* name);
int tcalc_context_has_func(const tcalc_context* context, const char* name);

int tcalc_context_has_op(const tcalc_context* context, const char* name);
int tcalc_context_has_unary_op(const tcalc_context* context, const char* name);
int tcalc_context_has_binary_op(const tcalc_context* context, const char* name);
int tcalc_context_has_unary_func(const tcalc_context* context, const char* name);
int tcalc_context_has_binary_func(const tcalc_context* context, const char* name);
int tcalc_context_has_variable(const tcalc_context* context, const char* name);

/**
 * tcalc_context_get_x functions will not return an error whenever the same given
 * context and name parameters return truthy from their corresponding tcalc_context_has_x
 * functions (if tcalc_context_has_x returns true, tcalc_context_get_x will not return an error)
*/

tcalc_error_t tcalc_context_get_unary_func(const tcalc_context* context, const char* name, tcalc_unary_funcdef** out);
tcalc_error_t tcalc_context_get_binary_func(const tcalc_context* context, const char* name, tcalc_binary_funcdef** out);
tcalc_error_t tcalc_context_get_variable(const tcalc_context* context, const char* name, tcalc_vardef** out);
tcalc_error_t tcalc_context_get_unary_op(const tcalc_context* context, const char* name, tcalc_unary_opdef** out);
tcalc_error_t tcalc_context_get_binary_op(const tcalc_context* context, const char* name, tcalc_binary_opdef** out);

tcalc_error_t tcalc_context_get_op_data(const tcalc_context* context, const char* name, tcalc_opdata* out);

tcalc_error_t tcalc_exprtree_node_alloc(tcalc_token* token, size_t nb_children, tcalc_exprtree** out);
void tcalc_exprtree_node_free(tcalc_exprtree* node);
void tcalc_exprtree_node_freev(void* node);

/**
 * Free a tcalc expression tree recursively
*/
void tcalc_exprtree_free(tcalc_exprtree* head);

int tcalc_exprtree_is_vardef(tcalc_exprtree* expr);

tcalc_error_t tcalc_create_exprtree_rpn(const char* rpn, const tcalc_context* context, tcalc_exprtree** out);
tcalc_error_t tcalc_create_exprtree_infix(const char* infix, const tcalc_context* context, tcalc_exprtree** out);
tcalc_error_t tcalc_infix_tokens_to_rpn_tokens(tcalc_token** tokens, size_t nb_tokens, const tcalc_context* context, tcalc_token*** out, size_t* out_size);

tcalc_error_t tcalc_eval_exprtree(tcalc_exprtree* expr, const tcalc_context* context, double* out);

#endif