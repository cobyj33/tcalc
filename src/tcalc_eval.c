#include "tcalc_eval.h"

#include "tcalc_context.h"

#include <malloc.h>



tcalc_error_t tcalc_eval(const char* infix, double* out) {
  tcalc_context* context;
  tcalc_error_t err = tcalc_ctx_alloc_default(&context);
  if (err) return err;

  err = tcalc_eval_wctx(infix, context, out);
  tcalc_ctx_free(context);
  return err;
}

tcalc_error_t tcalc_eval_wctx(const char* infix, tcalc_context* ctx, double* out) {
  tcalc_exprtree* tree;
  tcalc_error_t err = tcalc_create_exprtree_infix(infix, ctx, &tree);
  if (err) return err;

  err = tcalc_eval_exprtree(tree, ctx, out);
  tcalc_exprtree_free(tree);
  return err;
}

tcalc_error_t tcalc_eval_rpn(const char* rpn, double* out) {
  tcalc_context* context;
  tcalc_error_t err = tcalc_ctx_alloc_default(&context);
  if (err) return err;

  tcalc_exprtree* tree;
  err = tcalc_create_exprtree_rpn(rpn, context, &tree);
  if (err) {
    tcalc_ctx_free(context);
    return err;
  }

  err = tcalc_eval_exprtree(tree, context, out);
  tcalc_exprtree_free(tree);
  tcalc_ctx_free(context);
  return err;
}