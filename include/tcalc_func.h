#ifndef TCALC_FUNC_H
#define TCALC_FUNC_H

#include <stdbool.h>
#include "tcalc_error.h"

typedef tcalc_err (*tcalc_unfunc)(double, double*);
typedef tcalc_err (*tcalc_binfunc)(double, double, double*);
typedef bool (*tcalc_relfunc)(double, double);

// l suffix stands for "logical" in tcalc_unlfunc and tcalc_binlfunc

typedef bool (*tcalc_unlfunc)(bool);
typedef bool (*tcalc_binlfunc)(bool, bool);


// Logical Functions

bool tcalc_not(bool a);
bool tcalc_and(bool a, bool b);
bool tcalc_or(bool a, bool b);
bool tcalc_nand(bool a, bool b);
bool tcalc_nor(bool a, bool b);
bool tcalc_xor(bool a, bool b);
bool tcalc_xnor(bool a, bool b);
bool tcalc_matcond(bool a, bool b);
bool tcalc_equals_l(bool a, bool b);
bool tcalc_nequals_l(bool a, bool b);

// Relational Functions

bool tcalc_equals(double a, double b);
bool tcalc_nequals(double a, double b);
bool tcalc_lt(double a, double b);
bool tcalc_lteq(double a, double b);
bool tcalc_gt(double a, double b);
bool tcalc_gteq(double a, double b);


// Unary functions

tcalc_err tcalc_ceil(double a, double* out);
tcalc_err tcalc_floor(double a, double* out);
tcalc_err tcalc_round(double a, double* out);
tcalc_err tcalc_abs(double a, double* out);



// Radian Trigonometric Functions

tcalc_err tcalc_sin(double a, double* out);
tcalc_err tcalc_cos(double a, double* out);
tcalc_err tcalc_tan(double a, double* out);
tcalc_err tcalc_sec(double a, double* out);
tcalc_err tcalc_csc(double a, double* out);
tcalc_err tcalc_cot(double a, double* out);

tcalc_err tcalc_asin(double a, double* out);
tcalc_err tcalc_acos(double a, double* out);
tcalc_err tcalc_atan(double a, double* out);
tcalc_err tcalc_atan2(double a, double b, double* out);
tcalc_err tcalc_asec(double a, double* out);
tcalc_err tcalc_acsc(double a, double* out);
tcalc_err tcalc_acot(double a, double* out);

tcalc_err tcalc_sinh(double a, double* out);
tcalc_err tcalc_cosh(double a, double* out);
tcalc_err tcalc_tanh(double a, double* out);
tcalc_err tcalc_asinh(double a, double* out);
tcalc_err tcalc_acosh(double a, double* out);
tcalc_err tcalc_atanh(double a, double* out);

// Degree trigonometric functions

tcalc_err tcalc_sin_deg(double a, double* out);
tcalc_err tcalc_cos_deg(double a, double* out);
tcalc_err tcalc_tan_deg(double a, double* out);
tcalc_err tcalc_sec_deg(double a, double* out);
tcalc_err tcalc_csc_deg(double a, double* out);
tcalc_err tcalc_cot_deg(double a, double* out);

tcalc_err tcalc_asin_deg(double a, double* out);
tcalc_err tcalc_acos_deg(double a, double* out);
tcalc_err tcalc_atan_deg(double a, double* out);
tcalc_err tcalc_atan2_deg(double a, double b, double* out);
tcalc_err tcalc_asec_deg(double a, double* out);
tcalc_err tcalc_acsc_deg(double a, double* out);
tcalc_err tcalc_acot_deg(double a, double* out);

tcalc_err tcalc_sinh_deg(double a, double* out);
tcalc_err tcalc_cosh_deg(double a, double* out);
tcalc_err tcalc_tanh_deg(double a, double* out);
tcalc_err tcalc_asinh_deg(double a, double* out);
tcalc_err tcalc_acosh_deg(double a, double* out);
tcalc_err tcalc_atanh_deg(double a, double* out);


tcalc_err tcalc_unary_plus(double a, double* out);
tcalc_err tcalc_unary_minus(double a, double* out);
tcalc_err tcalc_log(double a, double* out);
tcalc_err tcalc_sqrt(double a, double* out);
tcalc_err tcalc_cbrt(double a, double* out);

tcalc_err tcalc_ln(double a, double* out);
tcalc_err tcalc_exp(double a, double* out);

// Binary functions:

tcalc_err tcalc_add(double a, double b, double* out);
tcalc_err tcalc_subtract(double a, double b, double* out);
tcalc_err tcalc_multiply(double a, double b, double* out);
tcalc_err tcalc_divide(double a, double b, double* out);
tcalc_err tcalc_mod(double a, double b, double* out);

tcalc_err tcalc_pow(double a, double b, double* out);

#endif
