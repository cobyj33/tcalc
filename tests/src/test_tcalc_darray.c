
#include "CuTest.h"
#include "tcalc_tests.h"
#include "tcalc_darray.h"

void TestTCalcDarray(CuTest *tc) {
  tcalc_darray* intdarray = tcalc_darray_alloc(sizeof(int));

  CuAssertPtrNotNull(tc, intdarray);
}

void TestTCalcDarrayPush(CuTest *tc) {
  tcalc_darray* intdarray = tcalc_darray_alloc(sizeof(int));
  CuAssertPtrNotNull(tc, intdarray);

  for (int i = 0; i < 100; i++) {
    tcalc_darray_push(intdarray, (void*)&i);
  }
  CuAssertTrue(tc, tcalc_darray_size(intdarray) == 100);

  int end;
  tcalc_darray_pop(intdarray, (void*)&end);
  CuAssertTrue(tc, end == 99);
}

CuSuite* TCalcDarrayGetSuite() {
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, TestTCalcDarray);
  SUITE_ADD_TEST(suite, TestTCalcDarrayPush);
  return suite;
}