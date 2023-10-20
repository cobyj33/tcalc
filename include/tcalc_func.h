#ifndef TCALC_FUNC_H
#define TCALC_FUNC_H

#include "tcalc_error.h"

int tcalc_equals(double a, double b);

tcalc_error_t tcalc_ceil(double a, double* out);
tcalc_error_t tcalc_floor(double a, double* out);
tcalc_error_t tcalc_round(double a, double* out);
tcalc_error_t tcalc_abs(double a, double* out);


tcalc_error_t tcalc_sin(double a, double* out);
tcalc_error_t tcalc_cos(double a, double* out);
tcalc_error_t tcalc_tan(double a, double* out);

tcalc_error_t tcalc_sec(double a, double* out);
tcalc_error_t tcalc_csc(double a, double* out);
tcalc_error_t tcalc_cot(double a, double* out);

tcalc_error_t tcalc_asin(double a, double* out);
tcalc_error_t tcalc_acos(double a, double* out);
tcalc_error_t tcalc_atan(double a, double* out);

tcalc_error_t tcalc_asec(double a, double* out);
tcalc_error_t tcalc_acsc(double a, double* out);
tcalc_error_t tcalc_acot(double a, double* out);

tcalc_error_t tcalc_sinh(double a, double* out);
tcalc_error_t tcalc_cosh(double a, double* out);
tcalc_error_t tcalc_tanh(double a, double* out);
tcalc_error_t tcalc_asinh(double a, double* out);
tcalc_error_t tcalc_acosh(double a, double* out);
tcalc_error_t tcalc_atanh(double a, double* out);


tcalc_error_t tcalc_unary_plus(double a, double* out);
tcalc_error_t tcalc_unary_minus(double a, double* out);

tcalc_error_t tcalc_add(double a, double b, double* out);
tcalc_error_t tcalc_subtract(double a, double b, double* out);
tcalc_error_t tcalc_multiply(double a, double b, double* out);
tcalc_error_t tcalc_divide(double a, double b, double* out);
tcalc_error_t tcalc_mod(double a, double b, double* out);

tcalc_error_t tcalc_pow(double a, double b, double* out);
tcalc_error_t tcalc_log(double a, double* out);
tcalc_error_t tcalc_sqrt(double a, double* out);
tcalc_error_t tcalc_cbrt(double a, double* out);

tcalc_error_t tcalc_ln(double a, double* out);
tcalc_error_t tcalc_exp(double a, double* out);

#endif