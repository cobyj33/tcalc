#include "tcalc_eval.h"

#include "tcalc_context.h"
#include "tcalc_exprtree.h"
#include "tcalc_error.h"
#include "tcalc_val.h"

#include <malloc.h>



tcalc_err tcalc_eval(const char* infix, tcalc_val* out) {
  tcalc_ctx* ctx;
  tcalc_err err = tcalc_ctx_alloc_default(&ctx);
  if (err) return err;

  err = tcalc_eval_wctx(infix, ctx, out);
  tcalc_ctx_free(ctx);
  return err;
}

tcalc_err tcalc_eval_wctx(const char* infix, const tcalc_ctx* ctx, tcalc_val* out) {
  tcalc_exprtree* tree;
  tcalc_err err = tcalc_create_exprtree_infix(infix, ctx, &tree);
  if (err) return err;

  err = tcalc_eval_exprtree(tree, ctx, out);
  tcalc_exprtree_free(tree);
  return err;
}

tcalc_err tcalc_eval_rpn(const char* infix, tcalc_val* out) {
  tcalc_ctx* ctx;
  tcalc_err err = TCALC_ERR_OK;

  ret_on_err(err, tcalc_ctx_alloc_default(&ctx));
  err = tcalc_eval_rpn_wctx(infix, ctx, out);
  tcalc_ctx_free(ctx);
  return err;
}

tcalc_err tcalc_eval_rpn_wctx(const char* rpn, const tcalc_ctx* ctx, tcalc_val* out) {
  tcalc_exprtree* tree;
  tcalc_err err = TCALC_ERR_OK;
  ret_on_err(err, tcalc_create_exprtree_rpn(rpn, ctx, &tree));

  err = tcalc_eval_exprtree(tree, ctx, out);
  tcalc_exprtree_free(tree);
  return err;
}
