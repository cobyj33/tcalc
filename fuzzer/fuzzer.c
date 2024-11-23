#include "tcalc.h"
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

static tcalc_token fuzzerTokenBuffer[TCALC_MIBI(1)];
static tcalc_exprtree fuzzerTreeNodeBuffer[TCALC_MIBI(1)];

// fuzz_target.cc
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  assert(Size < INT32_MAX);
  if (Size > INT32_MAX) return -1;
  int32_t dataLenI32 = (int32_t)Size;
  tcalc_val val = { 0 };
  int32_t tokensCount = 0, treeNodeCount = 0, exprRootInd = -1;
  tcalc_err err = tcalc_eval(
    (const char*)Data, dataLenI32,
    fuzzerTokenBuffer, (int32_t)TCALC_ARRAY_SIZE(fuzzerTokenBuffer),
    fuzzerTreeNodeBuffer, (int32_t)TCALC_ARRAY_SIZE(fuzzerTreeNodeBuffer),
    &val, &tokensCount, &treeNodeCount, &exprRootInd
  );
  return err == TCALC_ERR_OK ? 0 : -1;  // Values other than 0 and -1 are reserved for future use.
}
