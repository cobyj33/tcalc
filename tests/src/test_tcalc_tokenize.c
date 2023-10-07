#include "CuTest.h"
#include "tcalc_tests.h"
#include "tcalc_tokens.h"
#include "tcalc_dstring.h"
#include <stdio.h>
#include <stdlib.h>
#include "tcalc_error.h"
#include "tcalc_mem.h"

void TestTCalcSimpleTokens(CuTest *tc) {
  const char* expression = "3 .4 5.0 10 + - /";
  char** tokens;
  size_t size;
  tcalc_error_t err = tcalc_tokenize(expression, &tokens, &size);
  CuAssertTrue(tc, err == TCALC_OK);

  CuAssertIntEquals(tc, size, 7);

  CuAssertStrEquals(tc, tokens[0], "3");
  CuAssertStrEquals(tc, tokens[1], ".4");
  CuAssertStrEquals(tc, tokens[2], "5.0");
  CuAssertStrEquals(tc, tokens[3], "10");
  CuAssertStrEquals(tc, tokens[4], "+");
  CuAssertStrEquals(tc, tokens[5], "-");
  CuAssertStrEquals(tc, tokens[6], "/");


  tcalc_free_arr((void**)tokens, size, free);
}

CuSuite* TCalcTokenizeGetSuite() {
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, TestTCalcSimpleTokens);
  return suite;
}