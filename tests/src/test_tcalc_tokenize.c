#include "CuTest.h"
#include "tcalc_tests.h"
#include "tcalc_tokens.h"
#include <stdio.h>
#include <stdlib.h>
#include "tcalc_error.h"
#include "tcalc_mem.h"

void TestTCalcSimpleTokens(CuTest *tc) {
  const char* expression = "3 .4 5.0 10 + - /";
  tcalc_token_t** tokens;
  size_t size;
  tcalc_error_t err = tcalc_tokenize_rpn(expression, &tokens, &size);
  CuAssertTrue(tc, err == TCALC_OK);

  CuAssertIntEquals(tc, size, 7);

  CuAssertStrEquals(tc, tokens[0]->value, "3");
  CuAssertStrEquals(tc, tokens[1]->value, ".4");
  CuAssertStrEquals(tc, tokens[2]->value, "5.0");
  CuAssertStrEquals(tc, tokens[3]->value, "10");
  CuAssertStrEquals(tc, tokens[4]->value, "+");
  CuAssertStrEquals(tc, tokens[5]->value, "-");
  CuAssertStrEquals(tc, tokens[6]->value, "/");


  TCALC_ARR_FREE_F(tokens, size, tcalc_token_free);
}

CuSuite* TCalcTokenizeGetSuite() {
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, TestTCalcSimpleTokens);
  return suite;
}