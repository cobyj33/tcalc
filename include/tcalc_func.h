#ifndef TCALC_FUNC_H
#define TCALC_FUNC_H

#include "tcalc_error.h"

typedef tcalc_err (*tcalc_unfunc)(double, double*);
typedef tcalc_err (*tcalc_binfunc)(double, double, double*);
typedef int (*tcalc_relation_func)(double, double);

int tcalc_equals(double a, double b);
int tcalc_lt(double a, double b);
int tcalc_lteq(double a, double b);
int tcalc_gt(double a, double b);
int tcalc_gteq(double a, double b);

tcalc_err tcalc_ceil(double a, double* out);
tcalc_err tcalc_floor(double a, double* out);
tcalc_err tcalc_round(double a, double* out);
tcalc_err tcalc_abs(double a, double* out);


tcalc_err tcalc_sin(double a, double* out);
tcalc_err tcalc_cos(double a, double* out);
tcalc_err tcalc_tan(double a, double* out);

tcalc_err tcalc_sec(double a, double* out);
tcalc_err tcalc_csc(double a, double* out);
tcalc_err tcalc_cot(double a, double* out);

tcalc_err tcalc_asin(double a, double* out);
tcalc_err tcalc_acos(double a, double* out);
tcalc_err tcalc_atan(double a, double* out);

tcalc_err tcalc_asec(double a, double* out);
tcalc_err tcalc_acsc(double a, double* out);
tcalc_err tcalc_acot(double a, double* out);

tcalc_err tcalc_sinh(double a, double* out);
tcalc_err tcalc_cosh(double a, double* out);
tcalc_err tcalc_tanh(double a, double* out);
tcalc_err tcalc_asinh(double a, double* out);
tcalc_err tcalc_acosh(double a, double* out);
tcalc_err tcalc_atanh(double a, double* out);


tcalc_err tcalc_unary_plus(double a, double* out);
tcalc_err tcalc_unary_minus(double a, double* out);

tcalc_err tcalc_add(double a, double b, double* out);
tcalc_err tcalc_subtract(double a, double b, double* out);
tcalc_err tcalc_multiply(double a, double b, double* out);
tcalc_err tcalc_divide(double a, double b, double* out);
tcalc_err tcalc_mod(double a, double b, double* out);

tcalc_err tcalc_pow(double a, double b, double* out);
tcalc_err tcalc_log(double a, double* out);
tcalc_err tcalc_sqrt(double a, double* out);
tcalc_err tcalc_cbrt(double a, double* out);

tcalc_err tcalc_ln(double a, double* out);
tcalc_err tcalc_exp(double a, double* out);

#endif