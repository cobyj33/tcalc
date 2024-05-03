#include "tcalc_tests.h"

#include "CuTest.h"

#include "tcalc_context.h"
#include "tcalc_tokens.h"
#include "tcalc_error.h"
#include "tcalc_mem.h"

#include <stdio.h>
#include <stdlib.h>

void TestTCalcSimpleTokens(CuTest *tc) {
  const char* expression = "3 .4 5.0 10 + - /";
  tcalc_token** tokens;
  size_t size;
  tcalc_err err = tcalc_tokenize_rpn(expression, &tokens, &size);
  CuAssertTrue(tc, err == TCALC_ERR_OK);

  CuAssertIntEquals(tc, size, 7);

  CuAssertStrEquals(tc, tokens[0]->val, "3");
  CuAssertStrEquals(tc, tokens[1]->val, ".4");
  CuAssertStrEquals(tc, tokens[2]->val, "5.0");
  CuAssertStrEquals(tc, tokens[3]->val, "10");
  CuAssertStrEquals(tc, tokens[4]->val, "+");
  CuAssertStrEquals(tc, tokens[5]->val, "-");
  CuAssertStrEquals(tc, tokens[6]->val, "/");


  TCALC_ARR_FREE_F(tokens, size, tcalc_token_free);
}

CuSuite* TCalcTokenizeGetSuite() {
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, TestTCalcSimpleTokens);
  return suite;
}