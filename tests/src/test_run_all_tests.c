#include "tcalc_tests.h"

#include "CuTest.h"

#include <stdio.h>

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
