#include "tcalc_cli_progs.h"
#include "tcalc_cli_common.h"

#include "tcalc_error.h"
#include "tcalc_context.h"
#include "tcalc_eval.h"

#include <stdio.h>
#include <stdlib.h>


int tcalc_cli_eval_rpn(const char* expr, struct eval_opts eval_opts) {
  double ans;
  tcalc_ctx* ctx = NULL;
  tcalc_err err = tcalc_ctx_alloc_default(&ctx);
  if (err) {
    fprintf(stderr, "[tcalc_cli_eval_rpn] TCalc error while allocating evaluation context: %s\n ", tcalc_strerrcode(err));
    tcalc_errstk_printall();
    return EXIT_FAILURE;
  }

  if (!eval_opts.use_rads) {
    err = tcalc_ctx_addtrigdeg(ctx);
    if (err) {
      fprintf(stderr, "[tcalc_cli_eval_rpn] TCalc error while switching to degree-trig functions: %s\n ", tcalc_strerrcode(err));
      tcalc_errstk_printall();
      return EXIT_FAILURE;
    }
  }

  err = tcalc_eval_rpn_wctx(expr, ctx, &ans);

  if (err) {
    fprintf(stderr, "[tcalc_cli_eval_rpn] TCalc error while evaluating expression: %s\n ", tcalc_strerrcode(err));
    tcalc_errstk_printall();
    return EXIT_FAILURE;
  }

  printf("%f\n", ans);
  return EXIT_SUCCESS;
}
