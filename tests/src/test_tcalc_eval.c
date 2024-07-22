#include "tcalc_tests.h"

#include "tcalc_eval.h"
#include "tcalc_error.h"
#include "tcalc_val.h"

#include <stddef.h>
//tcalc_err tcalc_eval_rpn(const char* rpn, double* out);
#define TCALC_EVAL_ASSERT_DELTA 0.0001

void TestTCalcEvalSuccesses(CuTest *tc) {
  tcalc_val res = TCALC_VAL_INIT_NUM(0.0);
  tcalc_err err = TCALC_ERR_OK;

  #define MAKE_SUCCESS_TEST(str, val) \
    err = tcalc_eval(str, &res); \
    CuAssertTrue(tc, err == TCALC_ERR_OK); \
    CuAssertTrue(tc, res.type == TCALC_VALTYPE_NUM); \
    CuAssertDblEquals(tc, res.as.num, val, TCALC_EVAL_ASSERT_DELTA);

  MAKE_SUCCESS_TEST("6", 6.0);
  MAKE_SUCCESS_TEST("600", 600.0);
  MAKE_SUCCESS_TEST("2.53", 2.53);
  MAKE_SUCCESS_TEST(".253", .253);
  MAKE_SUCCESS_TEST(".253", .253);
  MAKE_SUCCESS_TEST("0.253", .253);
  MAKE_SUCCESS_TEST("0000.253", .253);
  MAKE_SUCCESS_TEST("   0000.253  ", .253);

  MAKE_SUCCESS_TEST("5 + 5", 10.0);
  MAKE_SUCCESS_TEST("5 + .5", 5.5);
  MAKE_SUCCESS_TEST("5 - 5", 0.0);
  MAKE_SUCCESS_TEST("5 * 5", 25.0);
  MAKE_SUCCESS_TEST("5 / 5", 1.0);
  MAKE_SUCCESS_TEST("5 % 5", 0.0);

  MAKE_SUCCESS_TEST("6 * 3 + 4 * ( 9 / 3 )", 30.0);
  MAKE_SUCCESS_TEST("2 + 6 * (4 + 5) / 3 - 5", 15.0);
  MAKE_SUCCESS_TEST("4 - 5 / (8 - 3) * 2 + 5", 7.0);
  MAKE_SUCCESS_TEST("100 / (6 + 7 * 2) - 5", 0.0);
  MAKE_SUCCESS_TEST("4 + (5 * 3 ^ 2 + 2)", 51.0);
  MAKE_SUCCESS_TEST("4 + (5 * 3 ** 2 + 2)", 51.0);
  MAKE_SUCCESS_TEST("9 - 24 / 8 * 2 + 3", 6.0);
  MAKE_SUCCESS_TEST("((32 / 4) + 3) * 2", 22.0);
  MAKE_SUCCESS_TEST("(3 * 5 ^ 2 / 5) - (16 - 10)", 9.0);
  MAKE_SUCCESS_TEST("-10 ^ 2", -100.0);
  MAKE_SUCCESS_TEST("-10 ** 2", -100.0);
  MAKE_SUCCESS_TEST("(-10) ^ 2", 100.0);
  MAKE_SUCCESS_TEST("(-10) ** 2", 100.0);
  MAKE_SUCCESS_TEST("2 ** 2 ^ 2 ** 2", 65536.0);
  MAKE_SUCCESS_TEST("5ln(e)", 5.0);
  MAKE_SUCCESS_TEST("2^2ln(e)", 4.0);
  MAKE_SUCCESS_TEST("2pi", 6.283185);
  MAKE_SUCCESS_TEST("pi(2)", 6.283185);
  MAKE_SUCCESS_TEST("e(pi)(2(4))", 68.317874);

  #undef MAKE_SUCCESS_TEST
}

void TestTCalcEvalFailures(CuTest *tc) {
  tcalc_val res = TCALC_VAL_INIT_NUM(0.0);

  CuAssertTrue(tc, tcalc_eval("1 / 0", &res) == TCALC_ERR_DIV_BY_ZERO);
  CuAssertTrue(tc, tcalc_eval("1 / -0", &res) == TCALC_ERR_DIV_BY_ZERO);
  CuAssertTrue(tc, tcalc_eval("0 / -0.0", &res) == TCALC_ERR_DIV_BY_ZERO);
  CuAssertTrue(tc, tcalc_eval("0 / 0.0", &res) == TCALC_ERR_DIV_BY_ZERO);

  CuAssertTrue(tc, tcalc_eval("unknownid", &res) == TCALC_ERR_UNKNOWN_ID);

  CuAssertTrue(tc, tcalc_eval("53.3.4", &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval(".53.3", &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval(".", &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval("", &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval("          ", &res) != TCALC_ERR_OK);
}

CuSuite* TCalcEvalGetSuite() {
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, TestTCalcEvalSuccesses);
  SUITE_ADD_TEST(suite, TestTCalcEvalFailures);
  return suite;
}
