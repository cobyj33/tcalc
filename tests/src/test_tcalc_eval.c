#include "tcalc_tests.h"

#include "tcalc.h"

#include <stddef.h>
#include <string.h>
#define TCALC_EVAL_ASSERT_DELTA 0.0001

static tcalc_err tcalc_eval_gb(const char* expr, int32_t exprLen, tcalc_val* out)
{
  int32_t dummyTreeNodeCount, dummyTokenCount;
  return tcalc_eval(
    expr,
    exprLen,
    globalTreeNodeBuffer,
    globalTreeNodeBufferCapacity,
    globalTokenBuffer,
    globalTokenBufferCapacity,
    out,
    &dummyTreeNodeCount,
    &dummyTokenCount
  );
}


void TestTCalcEvalSuccesses(CuTest *tc) {
  tcalc_token tokenBuffer[TCALC_KIBI(2)];
  tcalc_exprtree treeNodeBuffer[TCALC_KIBI(2)];

  #define MAKE_DOUBLE_SUCCESS_TEST(tc, expr, val) \
    { \
      tcalc_err err = TCALC_ERR_OK; \
      tcalc_val res = TCALC_VAL_INIT_NUM(0.0); \
      \
      err = tcalc_eval_gb(expr, (int32_t)strlen(expr), &res); \
      CuAssert_Line(tc, __FILE__, __LINE__, "Checking numerical expression '" expr "' evaluation for errors", err == TCALC_ERR_OK); \
      CuAssert_Line(tc, __FILE__, __LINE__, "Checking numerical expression '" expr "' returns as a double", res.type == TCALC_VALTYPE_NUM); \
      CuAssertDblEquals_LineMsg(tc, __FILE__, __LINE__, "Checking numerical expression '" expr "' evaluated to expected number", val, res.as.num, TCALC_EVAL_ASSERT_DELTA); \
    }

  #define MAKE_BOOL_SUCCESS_TEST(tc, expr, val) \
    { \
      tcalc_err err = TCALC_ERR_OK; \
      tcalc_val res = TCALC_VAL_INIT_BOOL(0); \
      \
      err = tcalc_eval_gb(expr, (int32_t)strlen(expr), &res); \
      CuAssert_Line(tc, __FILE__, __LINE__, "Checking boolean expression '" expr "' evaluation for errors", err == TCALC_ERR_OK); \
      CuAssert_Line(tc, __FILE__, __LINE__, "Checking boolean expression '" expr "' returns as a boolean", res.type == TCALC_VALTYPE_BOOL); \
      CuAssertIntEquals_LineMsg(tc, __FILE__, __LINE__, "Checking boolean xpression '" expr "' returns as expected boolean", !!val, !!res.as.boolean); \
    }

  // Double Tests

  // Expressions from the README
  MAKE_DOUBLE_SUCCESS_TEST(tc, "2 * 3 ^ ln(2)", 4.28297212);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "(sin(5))^2 + (cos(5))^2", 1.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "5 + sin(2 * pi)", 5.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "23 + arcsin(0.5) * (1 / 4)", 23.13089969);



  MAKE_DOUBLE_SUCCESS_TEST(tc, "6", 6.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "600", 600.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "2.53", 2.53);
  MAKE_DOUBLE_SUCCESS_TEST(tc, ".253", .253);
  MAKE_DOUBLE_SUCCESS_TEST(tc, ".253", .253);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "0.253", .253);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "0000.253", .253);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "   0000.253  ", .253);
  MAKE_DOUBLE_SUCCESS_TEST(tc, " -   0000.253 ( -1 ) ", .253);

  MAKE_DOUBLE_SUCCESS_TEST(tc, "5 + 5", 10.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "5 + .5", 5.5);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "5 - 5", 0.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "5 * 5", 25.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "5 / 5", 1.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "5 % 5", 0.0);

  MAKE_DOUBLE_SUCCESS_TEST(tc, "6 * 3 + 4 * ( 9 / 3 )", 30.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "2 + 6 * (4 + 5) / 3 - 5", 15.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "4 - 5 / (8 - 3) * 2 + 5", 7.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "100 / (6 + 7 * 2) - 5", 0.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "4 + (5 * 3 ^ 2 + 2)", 51.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "4 + (5 * 3 ** 2 + 2)", 51.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "9 - 24 / 8 * 2 + 3", 6.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "((32 / 4) + 3) * 2", 22.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "(3 * 5 ^ 2 / 5) - (16 - 10)", 9.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "-10 ^ 2", -100.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "-10 ** 2", -100.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "(-10) ^ 2", 100.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "(-10) ** 2", 100.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "2 ** 2 ^ 2 ** 2", 65536.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "5ln(e)", 5.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "2^2ln(e)", 4.0);
  MAKE_DOUBLE_SUCCESS_TEST(tc, "2pi", 6.283185);

  // Boolean Tests

  MAKE_BOOL_SUCCESS_TEST(tc, "true", 1);
  MAKE_BOOL_SUCCESS_TEST(tc, "false", 0);
  MAKE_BOOL_SUCCESS_TEST(tc, "true == false", 0);
  MAKE_BOOL_SUCCESS_TEST(tc, "true == true", 1);
  MAKE_BOOL_SUCCESS_TEST(tc, "false == true", 0);
  MAKE_BOOL_SUCCESS_TEST(tc, "false == false", 1);
  MAKE_BOOL_SUCCESS_TEST(tc, "true != false", 1);
  MAKE_BOOL_SUCCESS_TEST(tc, "true != true", 0);
  MAKE_BOOL_SUCCESS_TEST(tc, "false != true", 1);
  MAKE_BOOL_SUCCESS_TEST(tc, "false != false", 0);

  MAKE_BOOL_SUCCESS_TEST(tc, "true && true || false", 1);
  MAKE_BOOL_SUCCESS_TEST(tc, "true && (true || false)", 1);
  MAKE_BOOL_SUCCESS_TEST(tc, "false && true || true", 1);
  MAKE_BOOL_SUCCESS_TEST(tc, "(false && true) || true", 1);
  MAKE_BOOL_SUCCESS_TEST(tc, "false && (true || true)", 0);
  MAKE_BOOL_SUCCESS_TEST(tc, "false || true && true", 1);
  MAKE_BOOL_SUCCESS_TEST(tc, "false || !true && true", 0);
  MAKE_BOOL_SUCCESS_TEST(tc, "true || true && false", 1);
  MAKE_BOOL_SUCCESS_TEST(tc, "(true || true) && false", 0);
  MAKE_BOOL_SUCCESS_TEST(tc, "(5 <= 5) || (true || true) && false", 1);
  MAKE_BOOL_SUCCESS_TEST(tc, "(5 < 4) || (true || true) && false", 0);

  MAKE_BOOL_SUCCESS_TEST(tc, "101 == 101", 1);
  MAKE_BOOL_SUCCESS_TEST(tc, "10sin(pi) == 10sin(3pi)", 1);
  // TODO: Fix this case: MAKE_BOOL_SUCCESS_TEST(tc, "-2^3 < -2^5", 1);
  MAKE_BOOL_SUCCESS_TEST(tc, "2^3 < 2^5", 1);
}

void TestTCalcEvalFailures(CuTest *tc) {
  tcalc_token tokenBuffer[TCALC_KIBI(2)];
  tcalc_exprtree treeNodeBuffer[TCALC_KIBI(2)];
  tcalc_val res = TCALC_VAL_INIT_NUM(0.0);

  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("1 / 0"), &res) == TCALC_ERR_DIV_BY_ZERO);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("1 / -0"), &res) == TCALC_ERR_DIV_BY_ZERO);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("0 / -0.0"), &res) == TCALC_ERR_DIV_BY_ZERO);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("0 / 0.0"), &res) == TCALC_ERR_DIV_BY_ZERO);

  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("unknownid"), &res) == TCALC_ERR_UNKNOWN_ID);


  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN(""), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("\n"), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("a b c 10d e+"), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("53.3.4"), &res) != TCALC_ERR_OK);
  // TODO: Fix this case: CuAssertTrue(tc, tcalc_eval("53e4", &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN(".53.3"), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("."), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN(""), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("          "), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("          "), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("pi(2)"), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("e(pi)"), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("e(pi)(2(4))"), &res) != TCALC_ERR_OK);

  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("== 3"), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN(" ^^^^3--"), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN(" ^^^^3--"), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("+"), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("-"), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("=="), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("="), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("!"), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("/"), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN("("), &res) != TCALC_ERR_OK);
  CuAssertTrue(tc, tcalc_eval_gb(TCALC_STRLIT_PTR_LEN(")"), &res) != TCALC_ERR_OK);
}

CuSuite* TCalcEvalGetSuite() {
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, TestTCalcEvalSuccesses);
  SUITE_ADD_TEST(suite, TestTCalcEvalFailures);
  return suite;
}
