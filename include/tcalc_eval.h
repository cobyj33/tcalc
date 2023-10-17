
#ifndef TCALC_EVAL_H
#define TCALC_EVAL_H
#include "tcalc_error.h"

/**
 * tcalc_eval.h - the main meat
 * 
 * This is basically all the real application of tcalc is. It's pretty 
 * self-explanatory, at least I hope.
*/

tcalc_error_t tcalc_eval(const char* infix, double* out);
tcalc_error_t tcalc_eval_rpn(const char* rpn, double* out);

#endif