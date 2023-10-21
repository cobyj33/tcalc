#ifndef TCALC_TESTS_H
#define TCALC_TESTS_H

#include "CuTest.h"

#define TCALC_DBL_ASSERT_DELTA 0.001

CuSuite* TCalcEvalGetSuite();
CuSuite* TCalcRPNEvalGetSuite();
CuSuite* TCalcTokenizeGetSuite();

#endif