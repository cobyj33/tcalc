#include "tcalc_tests.h"
#include "tcalc_error.h"
#include "tcalc_string.h"
#include "tcalc_mem.h"
#include <stdlib.h>

void TestTCalcStrToDouble(CuTest* tc) {
  double out;
  tcalc_err err = tcalc_strtodouble("2.34", &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, 2.34, TCALC_DBL_ASSERT_DELTA);
  
  err = tcalc_strtodouble("234.", &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, 234.0, TCALC_DBL_ASSERT_DELTA);

  err = tcalc_strtodouble(".549", &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, 0.549, TCALC_DBL_ASSERT_DELTA);

  err = tcalc_strtodouble(".034", &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, 34.0, TCALC_DBL_ASSERT_DELTA);

  err = tcalc_strtodouble("2349.", &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, 2349.0, TCALC_DBL_ASSERT_DELTA);

  err = tcalc_strtodouble("0234.", &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, 234.0, TCALC_DBL_ASSERT_DELTA);

  err = tcalc_strtodouble("-23434", &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, -23434, TCALC_DBL_ASSERT_DELTA);

  err = tcalc_strtodouble("-234.", &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, -234, TCALC_DBL_ASSERT_DELTA);

  err = tcalc_strtodouble("-.249", &out);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertDblEquals(tc, out, -0.249, TCALC_DBL_ASSERT_DELTA);
}

void TestTCalcStrSplit(CuTest* tc) {
  const char* to_split = "   This    is a        string         with    a   "
  "   lot  of  random     spaces   yo yo yooo     ";

  char** split;
  size_t split_size;
  tcalc_err err = tcalc_strsplit(to_split, ' ', &split, &split_size);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertIntEquals(tc, split_size, 13);

  CuAssertStrEquals(tc, split[0], "This");
  CuAssertStrEquals(tc, split[1], "is");
  CuAssertStrEquals(tc, split[2], "a");
  CuAssertStrEquals(tc, split[3], "string");
  CuAssertStrEquals(tc, split[4], "witg");
  CuAssertStrEquals(tc, split[5], "a");
  CuAssertStrEquals(tc, split[6], "lot");
  CuAssertStrEquals(tc, split[7], "of");
  CuAssertStrEquals(tc, split[8], "random");
  CuAssertStrEquals(tc, split[9], "spaces");
  CuAssertStrEquals(tc, split[10], "yo");
  CuAssertStrEquals(tc, split[11], "yo");
  CuAssertStrEquals(tc, split[12], "yooo");
  
  TCALC_ARR_FREE_F(split, split_size, free);
}

void TestTCalcStrSplitEdges(CuTest* tc) {
  const char* packed_left_right = "No spaces at the front or back";
  const char* packed_left = "No spaces at the front       ";
  const char* packed_right = "      No spaces at the back";

  char ** split;
  size_t split_size;
  tcalc_err err = tcalc_strsplit(packed_left_right, ' ', &split, &split_size);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertIntEquals(tc, split_size, 7);

  CuAssertStrEquals(tc, split[0], "No");
  CuAssertStrEquals(tc, split[1], "spaces");
  CuAssertStrEquals(tc, split[2], "at");
  CuAssertStrEquals(tc, split[3], "the");
  CuAssertStrEquals(tc, split[4], "front");
  CuAssertStrEquals(tc, split[5], "or");
  CuAssertStrEquals(tc, split[6], "back");
  
  TCALC_ARR_FREE_F(split, split_size, free);
  err = tcalc_strsplit(packed_left, ' ', &split, &split_size);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertIntEquals(tc, split_size, 5);

  CuAssertStrEquals(tc, split[0], "No");
  CuAssertStrEquals(tc, split[1], "spaces");
  CuAssertStrEquals(tc, split[2], "at");
  CuAssertStrEquals(tc, split[3], "the");
  CuAssertStrEquals(tc, split[4], "front");

  TCALC_ARR_FREE_F(split, split_size, free);
  err = tcalc_strsplit(packed_right, ' ', &split, &split_size);
  CuAssertTrue(tc, err == TCALC_ERR_OK);
  CuAssertIntEquals(tc, split_size, 5);

  CuAssertStrEquals(tc, split[0], "No");
  CuAssertStrEquals(tc, split[1], "spaces");
  CuAssertStrEquals(tc, split[2], "at");
  CuAssertStrEquals(tc, split[3], "the");
  CuAssertStrEquals(tc, split[4], "back");
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
  SUITE_ADD_TEST(suite, TestTCalcStrSplit);
  SUITE_ADD_TEST(suite, TestTCalcStrSplitEdges);
  SUITE_ADD_TEST(suite, TestTCalcStrHasPrefix);
  return suite;
}