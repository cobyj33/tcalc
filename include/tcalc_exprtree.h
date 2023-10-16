#ifndef TCALC_PARSETREE_H
#define TCALC_PARSETREE_H

#include <stddef.h>
#include "tcalc_tokens.h"
#include "tcalc_error.h"

typedef struct tcalc_exprtree_node_t {
  tcalc_token_t* token;
  struct tcalc_exprtree_node_t** children;
  size_t nb_children;
} tcalc_exprtree_t;


tcalc_error_t tcalc_create_exprtree_rpn(const char* rpn, tcalc_exprtree_t** out);
tcalc_error_t tcalc_create_exprtree_infix(const char* infix, tcalc_exprtree_t** out);

/**
 * Free a tcalc expression tree **recursively**
*/
void tcalc_exprtree_free(tcalc_exprtree_t* head);



tcalc_error_t tcalc_eval_exprtree(tcalc_exprtree_t* expr, double* out);

#endif