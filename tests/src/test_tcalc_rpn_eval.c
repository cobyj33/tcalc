#include "CuTest.h"
#include "tcalc_tests.h"
#include "tcalc.h"
#include "tcalc_eval.h"
#include <stddef.h>
//tcalc_err tcalc_eval_rpn(const char* rpn, double* out);

double TCALC_RPN_EVAL_ASSERT_DELTA = 0.01;

void TestTCalcRPNEvalSuccesses(CuTest *tc) {
  double res = 0.0;
  tcalc_err err = tcalc_eval_rpn("2 4 +", &res);
  CuAssertTrue(tc, err == TCALC_OK);
  CuAssertDblEquals(tc, res, 6.0, TCALC_RPN_EVAL_ASSERT_DELTA);

  err = tcalc_eval_rpn("52 45 +", &res);
  CuAssertTrue(tc, err == TCALC_OK);
  CuAssertDblEquals(tc, res, 97.0, TCALC_RPN_EVAL_ASSERT_DELTA);
}

void TestTCalcRPNEvalFailures(CuTest *tc) {
  double res = 0.0;
  tcalc_err err = tcalc_eval_rpn("2 + 4", &res);
  CuAssertTrue(tc, err != TCALC_OK);

  err = tcalc_eval_rpn("+ 52 45", &res);
  CuAssertTrue(tc, err != TCALC_OK);
}

CuSuite* TCalcRPNEvalGetSuite() {
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, TestTCalcRPNEvalSuccesses);
  SUITE_ADD_TEST(suite, TestTCalcRPNEvalFailures);
  return suite;
}