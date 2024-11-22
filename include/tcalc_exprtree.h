#ifndef _TCALC_EXPRTREE_H_
#define _TCALC_EXPRTREE_H_

#include "tcalc_error.h"
#include "tcalc_tokens.h"

#include <stdbool.h>
#include <stddef.h>

struct tcalc_ctx;
struct tcalc_val;

typedef struct tcalc_exprtree_node tcalc_exprtree_node;
typedef struct tcalc_exprtree_node tcalc_exprtree;

// Valid Token Types:
// TCALC_TOK_ID: identifier for a binary function
// TCALC_TOK_BINOP: operator for a binary operator
// TCALC_TOK_BINLOP: operator for a binary logical operator
// TCALC_TOK_RELOP: operator for a binary relational operator
// TCALC_TOK_EQOP: operator for a binary equality operator
typedef struct tcalc_exprtree_binary_node {
  struct tcalc_token token;
  tcalc_exprtree_node* left;
  tcalc_exprtree_node* right;
} tcalc_exprtree_binary_node;

// Valid Token Types:
// TCALC_TOK_ID: identifier for a unary function
// TCALC_TOK_UNOP: operator for a unary operator
// TCALC_TOK_UNLOP: operator for a unary logical operator
typedef struct tcalc_exprtree_unary_node {
  struct tcalc_token token;
  tcalc_exprtree_node* child;
} tcalc_exprtree_unary_node;


// Valid Token Types:
// TCALC_TOK_ID: identifier for a variable
// TCALC_TOK_NUM: Numerical string
typedef struct tcalc_exprtree_value_node {
  struct tcalc_token token;
} tcalc_exprtree_value_node;


#if 0
// Currently Unused
// Exists in source code to show that the size of tcalc_exprtree_func_node when
// arbitrary sized functions are supported would be equal to the size of
// tcalc_exprtree_binary_node, making the union of the two waste no memory
typedef struct tcalc_exprtree_func_node {
  struct tcalc_token* token;
  struct tcalc_exprtree_node** children;
  size_t nb_children;
} tcalc_exprtree_func_node;
#endif

enum tcalc_exprtree_node_type {
  TCALC_EXPRTREE_NODE_TYPE_BINARY,
  TCALC_EXPRTREE_NODE_TYPE_UNARY,
  TCALC_EXPRTREE_NODE_TYPE_VALUE,
  // TCALC_EXPRTREE_NODE_TYPE_FUNC
};

struct tcalc_exprtree_node {
  enum tcalc_exprtree_node_type type;
  union {
    tcalc_exprtree_binary_node binary;
    tcalc_exprtree_unary_node unary;
    // tcalc_exprtree_func_node func;
    tcalc_exprtree_value_node value;
  } as;
};

/**
 * Free a tcalc expression tree **recursively**
 * This function can be called even if 'head' is NULL, any of the
 * children of 'head' are NULL, or the token of 'head' is NULL
*/
void tcalc_exprtree_free(tcalc_exprtree* head);

/**
 * Free a tcalc expression tree's children **recursively** and set each
 * child to NULL.
 *
 * defined as no-op if head == NULL
*/
void tcalc_exprtree_free_children(tcalc_exprtree* head);

tcalc_err tcalc_create_exprtree_infix(const char* expr, const struct tcalc_ctx* ctx, tcalc_exprtree** out);

tcalc_err tcalc_eval_exprtree(const char* expr, tcalc_exprtree* exprtree, const struct tcalc_ctx* ctx, struct tcalc_val* out);

#endif
