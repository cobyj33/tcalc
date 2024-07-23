#include "tcalc_tests.h"

#include "CuTest.h"

#include "tcalc.h"
#include "tcalc_val.h"
#include "tcalc_eval.h"
#include <stddef.h>
//tcalc_err tcalc_eval_rpn(const char* rpn, double* out);

double TCALC_RPN_EVAL_ASSERT_DELTA = 0.01;

#if 0
// TODO: Remove legacy code

void TestTCalcRPNEvalSuccesses(CuTest *tc) {
  tcalc_val res = TCALC_VAL_INIT_NUM(0.0);
  tcalc_err err = tcalc_eval_rpn("2 4 +", &res);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertTrue(tc, res.type == TCALC_VALTYPE_NUM);
  CuAssertDblEquals(tc, res.as.num, 6.0, TCALC_RPN_EVAL_ASSERT_DELTA);

  err = tcalc_eval_rpn("52 45 +", &res);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertTrue(tc, res.type == TCALC_VALTYPE_NUM);
  CuAssertDblEquals(tc, res.as.num, 97.0, TCALC_RPN_EVAL_ASSERT_DELTA);
}

void TestTCalcRPNEvalFailures(CuTest *tc) {
  tcalc_val res = TCALC_VAL_INIT_NUM(0.0);
  tcalc_err err = tcalc_eval_rpn("2 + 4", &res);
  CuAssertTrue(tc, err != TCALC_ERR_OK);

  err = tcalc_eval_rpn("+ 52 45", &res);
  CuAssertTrue(tc, err != TCALC_ERR_OK);
}

CuSuite* TCalcRPNEvalGetSuite() {
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, TestTCalcRPNEvalSuccesses);
  SUITE_ADD_TEST(suite, TestTCalcRPNEvalFailures);
  return suite;
}
#else
CuSuite* TCalcRPNEvalGetSuite() {
  CuSuite* suite = CuSuiteNew();
  return suite;
}
#endif
