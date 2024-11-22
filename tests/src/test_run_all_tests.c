#include "tcalc_tests.h"

#include "CuTest.h"

#include <stdio.h>

tcalc_token globalTokenBuffer[TCALC_KIBI(2)];
int32_t globalTokenBufferCapacity = (int32_t)TCALC_ARRAY_SIZE(globalTokenBuffer);
int32_t globalTokenBufferLen;

tcalc_exprtree globalTreeNodeBuffer[TCALC_KIBI(2)];
int32_t globalTreeNodeBufferCapacity = (int32_t)TCALC_ARRAY_SIZE(globalTreeNodeBuffer);
int32_t globalTreeNodeBufferLen;

void RunAllTests() {
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();

    CuSuiteAddSuite(suite, TCalcEvalGetSuite());
    CuSuiteAddSuite(suite, TCalcTokenizeGetSuite());

    CuSuiteRun(suite);

    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}

int main(void) {
    RunAllTests();
}
