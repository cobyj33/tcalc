#include "tcalc_tests.h"

#include "tcalc_eval.h"

#include "CuTest.h"

#include <stddef.h>
//tcalc_error_t tcalc_eval_rpn(const char* rpn, double* out);

double TCALC_EVAL_ASSERT_DELTA = 0.0001;

void TestTCalcEvalSuccesses(CuTest *tc) {
  double res = 0.0;
  tcalc_error_t err = tcalc_eval("6 * 3 + 4 * ( 9 / 3 )", &res);
  CuAssertTrue(tc, err == TCALC_OK);
  CuAssertDblEquals(tc, res, 30.0, TCALC_EVAL_ASSERT_DELTA);

  err = tcalc_eval("2 + 6 * (4 + 5) / 3 - 5", &res);
  CuAssertTrue(tc, err == TCALC_OK);
  CuAssertDblEquals(tc, res, 15.0, TCALC_EVAL_ASSERT_DELTA);

  err = tcalc_eval("4 - 5 / (8 - 3) * 2 + 5", &res);
  CuAssertTrue(tc, err == TCALC_OK);
  CuAssertDblEquals(tc, res, 7.0, TCALC_EVAL_ASSERT_DELTA);

  err = tcalc_eval("100 / (6 + 7 * 2) - 5", &res);
  CuAssertTrue(tc, err == TCALC_OK);
  CuAssertDblEquals(tc, res, 0.0, TCALC_EVAL_ASSERT_DELTA);

  err = tcalc_eval("4 + (5 * 3 ^ 2 + 2)", &res);
  CuAssertTrue(tc, err == TCALC_OK);
  CuAssertDblEquals(tc, res, 51.0, TCALC_EVAL_ASSERT_DELTA);

  err = tcalc_eval("9 - 24 / 8 * 2 + 3", &res);
  CuAssertTrue(tc, err == TCALC_OK);
  CuAssertDblEquals(tc, res, 6.0, TCALC_EVAL_ASSERT_DELTA);

  err = tcalc_eval("((32 / 4) + 3) * 2", &res);
  CuAssertTrue(tc, err == TCALC_OK);
  CuAssertDblEquals(tc, res, 22.0, TCALC_EVAL_ASSERT_DELTA);

  err = tcalc_eval("(3 * 5 ^ 2 / 5) - (16 - 10)", &res);
  CuAssertTrue(tc, err == TCALC_OK);
  CuAssertDblEquals(tc, res, 9.0, TCALC_EVAL_ASSERT_DELTA);

  err = tcalc_eval("-10 ^ 2", &res);
  CuAssertTrue(tc, err == TCALC_OK);
  CuAssertDblEquals(tc, res, -100.0, TCALC_EVAL_ASSERT_DELTA);

  err = tcalc_eval("(-10) ^ 2", &res);
  CuAssertTrue(tc, err == TCALC_OK);
  CuAssertDblEquals(tc, res, 100.0, TCALC_EVAL_ASSERT_DELTA);

  err = tcalc_eval("5ln(e)", &res);
  CuAssertTrue(tc, err == TCALC_OK);
  CuAssertDblEquals(tc, res, 5.0, TCALC_EVAL_ASSERT_DELTA);

  err = tcalc_eval("2^2ln(e)", &res);
  CuAssertTrue(tc, err == TCALC_OK);
  CuAssertDblEquals(tc, res, 4.0, TCALC_EVAL_ASSERT_DELTA);

  err = tcalc_eval("2pi", &res);
  CuAssertTrue(tc, err == TCALC_OK);
  CuAssertDblEquals(tc, res, 6.283185, TCALC_EVAL_ASSERT_DELTA);
}

void TestTCalcEvalFailures(CuTest *tc) {
  double res;
  tcalc_error_t err = tcalc_eval("1 / sin(2 * pi)", &res);
  CuAssertTrue(tc, err == TCALC_DIVISION_BY_ZERO);
}

CuSuite* TCalcEvalGetSuite() {
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, TestTCalcEvalSuccesses);
  SUITE_ADD_TEST(suite, TestTCalcEvalFailures);
  return suite;
}