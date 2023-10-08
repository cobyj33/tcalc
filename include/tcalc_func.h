#ifndef TCALC_FUNC_H
#define TCALC_FUNC_H

#include "tcalc_error.h"

typedef tcalc_error_t (*tcalc_binary_op_func)(double, double, double*);

tcalc_error_t tcalc_add(double a, double b, double* out);

tcalc_error_t tcalc_subtract(double a, double b, double* out);

tcalc_error_t tcalc_multiply(double a, double b, double* out);

tcalc_error_t tcalc_divide(double a, double b, double* out);

tcalc_error_t tcalc_pow(double a, double b, double* out);

#endif