#include "tcalc_tests.h"

#include "CuTest.h"

#include "tcalc_context.h"
#include "tcalc_tokens.h"
#include "tcalc_error.h"
#include "tcalc_mem.h"

#include <stdio.h>
#include <stdlib.h>

CuSuite* TCalcTokenizeGetSuite() {
  CuSuite* suite = CuSuiteNew();
  return suite;
}
