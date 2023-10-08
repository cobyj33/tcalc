#ifndef TCALC_PARSETREE_H
#define TCALC_PARSETREE_H

#include <stddef.h>
#include "tcalc_tokens.h"
#include "tcalc_error.h"

typedef struct {
  tcalc_token_t token;
  struct tcalc_expression_t* children;
  size_t nb_children;
} tcalc_expression_t;

tcalc_error_t tcalc_eval_expression_tree(tcalc_expression_t* expr, double* out);

#endif