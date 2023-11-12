#include "tcalc_eval.h"

#include "tcalc_context.h"

#include <malloc.h>

tcalc_error_t tcalc_eval(const char* infix, double* out) {
  tcalc_context_t* context;
  tcalc_error_t err = tcalc_context_alloc_default(&context);
  if (err) return err;

  tcalc_exprtree_t* tree;
  err = tcalc_create_exprtree_infix(infix, context, &tree);
  if (err) {
    tcalc_context_free(context);
    return err;
  }

  err = tcalc_eval_exprtree(tree, context, out);
  tcalc_exprtree_free(tree);
  tcalc_context_free(context);
  return err;
}

tcalc_error_t tcalc_eval_rpn(const char* rpn, double* out) {
  tcalc_context_t* context;
  tcalc_error_t err = tcalc_context_alloc_default(&context);
  if (err) return err;

  tcalc_exprtree_t* tree;
  err = tcalc_create_exprtree_rpn(rpn, context, &tree);
  if (err) {
    tcalc_context_free(context);
    return err;
  }

  err = tcalc_eval_exprtree(tree, context, out);
  tcalc_exprtree_free(tree);
  tcalc_context_free(context);
  return err;
}