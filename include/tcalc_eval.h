
#ifndef TCALC_EVAL_H
#define TCALC_EVAL_H
#include "tcalc_error.h"

tcalc_error_t tcalc_eval(const char* infix, double* out);
tcalc_error_t tcalc_eval_rpn(const char* infix, double* out);

#endif