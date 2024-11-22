#include "tcalc_tests.h"

#include "CuTest.h"

#include "tcalc.h"

#include <stdio.h>
#include <stdlib.h>

CuSuite* TCalcTokenizeGetSuite() {
  CuSuite* suite = CuSuiteNew();
  return suite;
}
