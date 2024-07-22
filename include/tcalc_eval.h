
#ifndef TCALC_EVAL_H
#define TCALC_EVAL_H

#include "tcalc_error.h"

struct tcalc_ctx;
struct tcalc_val;

/**
 * tcalc_eval.h - the main meat
 *
 * This is basically all the real application of tcalc is. Everything else is to
 * make these two functions work well. They're quite simple to be honest, they do math.
*/

tcalc_err tcalc_eval_wctx(const char* infix, const struct tcalc_ctx* ctx, struct tcalc_val* out);
tcalc_err tcalc_eval(const char* infix, struct tcalc_val* out);

tcalc_err tcalc_eval_rpn_wctx(const char* rpn, const struct tcalc_ctx* ctx, struct tcalc_val* out);
tcalc_err tcalc_eval_rpn(const char* rpn, struct tcalc_val* out);

#endif
