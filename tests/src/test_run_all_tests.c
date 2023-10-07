#include "CuTest.h"
#include "tcalc_tests.h"

#include <stdio.h>

void RunAllTests() {
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();
    
    CuSuiteAddSuite(suite, TCalcDarrayGetSuite());
    CuSuiteAddSuite(suite, TCalcRPNEvalGetSuite());
    CuSuiteAddSuite(suite, TCalcTokenizeGetSuite());
    CuSuiteAddSuite(suite, TCalcDStringSuite());

    CuSuiteRun(suite);

    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}

int main(void) {
    RunAllTests();
}
