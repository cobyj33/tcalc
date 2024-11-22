#include "tcalc_tests.h"
#include "tcalc.h"
#include <stdlib.h>

void TestTCalcStrToDouble(CuTest* tc) {
  double out;
  tcalc_err err = tcalc_lpstrtodouble(TCALC_STRLIT_PTR_LEN("2.34"), &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, 2.34, TCALC_DBL_ASSERT_DELTA);

  err = tcalc_lpstrtodouble(TCALC_STRLIT_PTR_LEN("234."), &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, 234.0, TCALC_DBL_ASSERT_DELTA);

  err = tcalc_lpstrtodouble(TCALC_STRLIT_PTR_LEN(".549"), &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, 0.549, TCALC_DBL_ASSERT_DELTA);

  err = tcalc_lpstrtodouble(TCALC_STRLIT_PTR_LEN(".034"), &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, 34.0, TCALC_DBL_ASSERT_DELTA);

  err = tcalc_lpstrtodouble(TCALC_STRLIT_PTR_LEN("2349."), &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, 2349.0, TCALC_DBL_ASSERT_DELTA);

  err = tcalc_lpstrtodouble(TCALC_STRLIT_PTR_LEN("0234."), &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, 234.0, TCALC_DBL_ASSERT_DELTA);

  err = tcalc_lpstrtodouble(TCALC_STRLIT_PTR_LEN("-23434"), &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, -23434, TCALC_DBL_ASSERT_DELTA);

  err = tcalc_lpstrtodouble(TCALC_STRLIT_PTR_LEN("-234."), &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, -234, TCALC_DBL_ASSERT_DELTA);

  err = tcalc_lpstrtodouble(TCALC_STRLIT_PTR_LEN("-.249"), &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, -0.249, TCALC_DBL_ASSERT_DELTA);
}

void TestTCalcStrHasPrefix(CuTest* tc) {
  CuAssertTrue(tc, tcalc_strhaspre("**", "**5+3"));
  CuAssertTrue(tc, tcalc_strhaspre("pre", "prefix"));
  CuAssertTrue(tc, tcalc_strhaspre("", ""));
  CuAssertTrue(tc, tcalc_strhaspre("", "trivial"));


  CuAssertTrue(tc, !tcalc_strhaspre("**", "*"));
  CuAssertTrue(tc, !tcalc_strhaspre("prefix", "pre"));
  CuAssertTrue(tc, !tcalc_strhaspre("!()", "!(trivial"));
  CuAssertTrue(tc, !tcalc_strhaspre("trivial", ""));
}

CuSuite* TCalcStringSuite() {
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, TestTCalcStrToDouble);
  SUITE_ADD_TEST(suite, TestTCalcStrHasPrefix);
  return suite;
}
