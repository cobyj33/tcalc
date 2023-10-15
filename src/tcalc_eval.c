#include "tcalc_eval.h"

#include "tcalc_exprtree.h"

#include <malloc.h>

tcalc_error_t tcalc_eval(const char* infix, double* out) {
  tcalc_exprtree_t* tree;
  tcalc_error_t err = tcalc_create_exprtree_infix(infix, &tree);
  if (err) return err;

  err = tcalc_eval_exprtree(tree, out);
  tcalc_exprtree_free(tree);
  return err;
}

tcalc_error_t tcalc_eval_rpn(const char* rpn, double* out) {
  tcalc_exprtree_t* tree;
  tcalc_error_t err = tcalc_create_exprtree_rpn(rpn, &tree);
  if (err) return err;

  err = tcalc_eval_exprtree(tree, out);
  tcalc_exprtree_free(tree);
  return err;
}