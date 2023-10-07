#include "tcalc_tests.h"
#include "tcalc_dstring.h"
#include "string.h"

// void TestTCalcDStringStrlcpy(CuTest *tc) {

// }

void TestTCalcDStringSetCSubStr(CuTest *tc) {
  tcalc_dstring* str = tcalc_dstring_alloc();
  const char* digits = "0123456789";

  tcalc_dstring_set_csubstr(str, digits, 0, 4);
  CuAssertIntEquals(tc, tcalc_dstring_size(str), 4);
  CuAssertIntEquals(tc, tcalc_dstring_strcmp(str, "0123"), 0);
  
  // longer string to shorter string
  tcalc_dstring_set_csubstr(str, digits, 0, 2);
  CuAssertIntEquals(tc, tcalc_dstring_size(str), 2);
  CuAssertIntEquals(tc, tcalc_dstring_strcmp(str, "01"), 0);

  // shorter string to longer string
  tcalc_dstring_set_csubstr(str, digits, 0, 7);
  CuAssertIntEquals(tc, tcalc_dstring_size(str), 7);
  CuAssertIntEquals(tc, tcalc_dstring_strcmp(str, "0123456"), 0);
  
  // to empty string
  tcalc_dstring_set_csubstr(str, digits, 0, 0);
  CuAssertIntEquals(tc, tcalc_dstring_size(str), 0);
  CuAssertIntEquals(tc, tcalc_dstring_strcmp(str, ""), 0);

  const char* long_string = "This is just a really long string to make sure "
  "that the dstring won't overflow and will properly reallocate itself to accomodate "
  "for the substantially larger space that a longer string would take up. On the "
  "other hand, I hope you're all having a nice day today!";
  tcalc_dstring_set_csubstr(str, long_string, 0, strlen(long_string));
  CuAssertIntEquals(tc, tcalc_dstring_size(str), strlen(long_string));
  CuAssertIntEquals(tc, tcalc_dstring_strcmp(str, long_string), 0);
}

CuSuite* TCalcDStringSuite() {
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, TestTCalcDStringSetCSubStr);
  return suite;
}