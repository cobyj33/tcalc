#ifndef TCALC_PARSETREE_H
#define TCALC_PARSETREE_H

#include <stddef.h>
#include "tcalc_tokens.h"
#include "tcalc_error.h"

typedef struct {
  tcalc_token_t token;
  struct tcalc_exprtree_t** children;
  size_t nb_children;
} tcalc_exprtree_t;

tcalc_error_t tcalc_create_exprtree_rpn(const char* rpn, tcalc_exprtree_t** out);
tcalc_error_t tcalc_create_exprtree_infix(const char* rpn, tcalc_exprtree_t** out);

tcalc_error_t tcalc_eval_exprtree(tcalc_exprtree_t* expr, double* out);

tcalc_error_t tcalc_exprtree_to_rpn(tcalc_exprtree_t* expr, char** out);

#endif