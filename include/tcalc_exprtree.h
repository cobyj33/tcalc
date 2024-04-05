#ifndef _TCALC_EXPRTREE_H_
#define _TCALC_EXPRTREE_H_

#include "tcalc_error.h"
#include <stddef.h>

struct tcalc_token;
struct tcalc_ctx;

typedef enum tcalc_exprtype {
  TCALC_RELATION_EXPR,
  TCALC_ARITHMETIC_EXPR
} tcalc_exprtype;

typedef struct tcalc_algres {
  tcalc_exprtype exprtype;
  union {
    double num;
    int boolean;
  } retval;
} tcalc_algres;

/**
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
  struct tcalc_token* token;
  struct tcalc_exprtree_node** children;
  size_t nb_children;
} tcalc_exprtree;

/**
 * tcalc_reltree_node:
 * Defines a tree-structure of the relationship between two algebraic expressions
 * Connects to multiple tcalc_exprtree_node instances
*/
typedef struct tcalc_reltree_node {
  struct tcalc_token* token;
  struct tcalc_exprtree_node** children;
  size_t nb_children;
} tcalc_reltree;

/**
 * tcalc_logtree_node:
 * Defines a tree-structure of the relationship between two relational expressions
 * 
*/
typedef struct tcalc_logtree_node {
  struct tcalc_token* token;
  struct tcalc_reltree_node** children;
  size_t nb_children;
} tcalc_logtree;

typedef enum tcalc_algtree_type {
  TCALC_ALGTREE_ARITH,
  TCALC_ALGTREE_REL,
  TCALC_ALGTREE_LOGIC,
} tcalc_algtree_type;

/**
 * tcalc_algtree_node:
 * 
 * A n-ary tree data structure representing any sort of algebraic expression,
 * whether that be an arithmetic expression, relational expression, or a
 * logical expression
*/
typedef struct tcalc_algtree_node {
  tcalc_algtree_type type;
  union {
    tcalc_exprtree* expr;
    tcalc_reltree* rel;
    tcalc_logtree* log;
  } node;
} tcalc_algtree_node;

tcalc_err tcalc_exprtree_node_alloc(struct tcalc_token* token, size_t nb_children, tcalc_exprtree** out);
void tcalc_exprtree_node_free(tcalc_exprtree* node);

/**
 * Free a tcalc expression tree **recursively**
*/
void tcalc_exprtree_free(tcalc_exprtree* head);

/**
 * Vardef form: 
 *  <variable> "=" expression
*/
int tcalc_exprtree_is_vardef(tcalc_exprtree* expr);

tcalc_err tcalc_create_exprtree_rpn(const char* rpn, const struct tcalc_ctx* ctx, tcalc_exprtree** out);
tcalc_err tcalc_create_exprtree_infix(const char* infix, const struct tcalc_ctx* ctx, tcalc_exprtree** out);
tcalc_err tcalc_infix_tokens_to_rpn_tokens(struct tcalc_token** tokens, size_t nb_tokens, const struct tcalc_ctx* ctx, struct tcalc_token*** out, size_t* out_size);

tcalc_err tcalc_eval_exprtree(tcalc_exprtree* expr, const struct tcalc_ctx* ctx, double* out);

#endif