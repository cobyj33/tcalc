#include "tcalc_cli_progs.h"
#include "tcalc_cli_common.h"

#include "tcalc.h"

#include <stdio.h>
#include <stdlib.h>


int tcalc_cli_eval(const char* expr, int32_t exprLen, struct eval_opts eval_opts) {
  tcalc_val ans;
  tcalc_ctx* ctx = NULL;
  tcalc_err err = tcalc_ctx_alloc_default(&ctx);
  TCALC_CLI_CHECK_ERR(err, "[%s] TCalc error while allocating evaluation context: %s\n", __func__, tcalc_strerrcode(err));

  if (!eval_opts.use_rads) {
    err = tcalc_ctx_addtrigdeg(ctx);
    TCALC_CLI_CHECK_ERR(err, "[%s] TCalc error while switching to degree-trig functions: %s\n ", __func__, tcalc_strerrcode(err));
  }

  int32_t treeNodeCount = 0, tokenCount = 0;
  err = tcalc_eval_wctx(
    expr, exprLen, globalTreeNodeBuffer, globalTreeNodeBufferCapacity,
    globalTokenBuffer, globalTokenBufferCapacity, ctx, &ans,
    &treeNodeCount, &tokenCount
  );

  TCALC_CLI_CHECK_ERR(err, "[%s] TCalc error while evaluating expression: %s\n ", __func__, tcalc_strerrcode(err));

  tcalc_val_fputline(stdout, ans);
  return EXIT_SUCCESS;
}
