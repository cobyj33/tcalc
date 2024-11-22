#ifndef TCALC_TESTS_H
#define TCALC_TESTS_H

#include "CuTest.h"

#include "tcalc.h"

extern tcalc_token globalTokenBuffer[];
extern int32_t globalTokenBufferCapacity;
extern int32_t globalTokenBufferLen;

extern tcalc_exprtree globalTreeNodeBuffer[];
extern int32_t globalTreeNodeBufferCapacity;
extern int32_t globalTreeNodeBufferLen;

#define TCALC_DBL_ASSERT_DELTA 0.001

CuSuite* TCalcEvalGetSuite();
CuSuite* TCalcTokenizeGetSuite();

#endif
