#include "tcalc_cli_progs.h"
#include "tcalc_cli_common.h"

#include "tcalc_error.h"
#include "tcalc_context.h"
#include "tcalc_eval.h"
#include "tcalc_mac.h"
#include "tcalc_val.h"

#include <stdio.h>
#include <stdlib.h>

int tcalc_cli_eval(const char* expr, struct eval_opts eval_opts) {
  tcalc_val ans;
  tcalc_ctx* ctx = NULL;
  tcalc_err err = tcalc_ctx_alloc_default(&ctx);
  TCALC_CLI_CHECK_ERR(err, "[%s] TCalc error while allocating evaluation context: %s\n", FUNCDINFO, tcalc_strerrcode(err));

  if (!eval_opts.use_rads) {
    err = tcalc_ctx_addtrigdeg(ctx);
    TCALC_CLI_CHECK_ERR(err, "[%s] TCalc error while switching to degree-trig functions: %s\n ", FUNCDINFO, tcalc_strerrcode(err));
  }

  err = tcalc_eval_wctx(expr, ctx, &ans);
  TCALC_CLI_CHECK_ERR(err, "[%s] TCalc error while evaluating expression: %s\n ", FUNCDINFO, tcalc_strerrcode(err));

  tcalc_val_fput(ans, stdout);
  fputc('\n', stdout);

  return EXIT_SUCCESS;
}
