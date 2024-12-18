#include "tcalc.h"

#include <assert.h>

tcalc_err tcalc_eval(
  const char* expr, int32_t exprLen,
  tcalc_exprtree* treeNodesBuffer, int32_t treeNodesBufferCapacity,
  tcalc_token* tokensBuffer, int32_t tokensBufferCapacity,
  struct tcalc_val* out, int32_t *outTreeNodesCount, int32_t *outTokensCount
) {
  assert(out != NULL);
  assert(outTreeNodesCount != NULL);
  assert(outTokensCount != NULL);
  *out = (struct tcalc_val){ 0 };
  *outTreeNodesCount = 0;
  *outTokensCount = 0;

  tcalc_ctx* ctx = NULL;
  tcalc_err err = tcalc_ctx_alloc_default(&ctx);
  if (err) return err;

  err = tcalc_eval_wctx(
    expr, exprLen, treeNodesBuffer, treeNodesBufferCapacity,
    tokensBuffer, tokensBufferCapacity, ctx, out,
    outTreeNodesCount, outTokensCount
  );

  tcalc_ctx_free(ctx);
  return err;
}

tcalc_err tcalc_eval_wctx(
  const char* expr, int32_t exprLen,
  tcalc_exprtree* treeNodesBuffer, int32_t treeNodesBufferCapacity,
  tcalc_token* tokensBuffer, int32_t tokensBufferCapacity,
  const struct tcalc_ctx* ctx,
  struct tcalc_val* out, int32_t *outTreeNodesCount, int32_t *outTokensCount
) {
  assert(out != NULL);
  assert(outTreeNodesCount != NULL);
  assert(outTokensCount != NULL);
  *out = (struct tcalc_val){ 0 };
  *outTreeNodesCount = 0;
  *outTokensCount = 0;

  tcalc_err err = TCALC_ERR_OK;
  int32_t tokensCount = 0;
  err = tcalc_tokenize_infix(
    expr, exprLen, tokensBuffer, tokensBufferCapacity, &tokensCount
  );
  if (err) return err;

  int32_t treeNodesCount = 0;
  int32_t exprRootInd = 0;
  err = tcalc_create_exprtree_infix(
    expr, exprLen, tokensBuffer, tokensCount,
    treeNodesBuffer, treeNodesBufferCapacity, &treeNodesCount, &exprRootInd
  );
  if (err) return err;

  err = tcalc_eval_exprtree(
    expr, exprLen, treeNodesBuffer, treeNodesCount,
    exprRootInd, tokensBuffer, tokensCount, ctx, out
  );
  *outTreeNodesCount = treeNodesCount;
  *outTokensCount = tokensCount;
  return err;
}
