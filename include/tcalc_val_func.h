#ifndef TCALC_VAL_FUNC_H
#define TCALC_VAL_FUNC_H

#include "tcalc_error.h"
#include "tcalc_val.h"

// Note how each tcalc_val function does not return a tcalc_val through
// its out parameter, but instead returns a C-type corresponding to the
// necessary return type for that function.

// Do note that the 'out' parameter can still connect directly to a
// member of a tcalc_val struct, which would be the most useful way
// to use these functions. For example:
//
//   struct tcalc_val a = TCALC_VAL_INIT_NUM(500);
//   struct tcalc_val b = TCALC_VAL_INIT_NUM(300);
//   struct tcalc_val out = { .type = TCALC_VALTYPE_NUM };
//   tcalc_err err = tcalc_val_and(a, b, &(out.as.num));
//   if (err) return err;
//   ..

typedef tcalc_err (*tcalc_val_unfunc)(tcalc_val, double*);
typedef tcalc_err (*tcalc_val_binfunc)(tcalc_val, tcalc_val, double*);
typedef tcalc_err (*tcalc_val_relfunc)(tcalc_val, tcalc_val, int*);

// l suffix stands for "logical"
typedef tcalc_err (*tcalc_val_unlfunc)(tcalc_val, int* out);
typedef tcalc_err (*tcalc_val_binlfunc)(tcalc_val, tcalc_val, int* out);


/**
 * Logical Functions
*/
tcalc_err tcalc_val_not(tcalc_val a, int* out);
tcalc_err tcalc_val_and(tcalc_val a, tcalc_val b, int* out);
tcalc_err tcalc_val_or(tcalc_val a, tcalc_val b, int* out);
tcalc_err tcalc_val_nand(tcalc_val a, tcalc_val b, int* out);
tcalc_err tcalc_val_nor(tcalc_val a, tcalc_val b, int* out);
tcalc_err tcalc_val_xor(tcalc_val a, tcalc_val b, int* out);
tcalc_err tcalc_val_xnor(tcalc_val a, tcalc_val b, int* out);
tcalc_err tcalc_val_matcond(tcalc_val a, tcalc_val b, int* out);
tcalc_err tcalc_val_equals_l(tcalc_val a, tcalc_val b, int* out);
tcalc_err tcalc_val_nequals_l(tcalc_val a, tcalc_val b, int* out);

/**
 * Relational Functions
*/
tcalc_err tcalc_val_equals(tcalc_val a, tcalc_val b, int* out);
tcalc_err tcalc_val_nequals(tcalc_val a, tcalc_val b, int* out);
tcalc_err tcalc_val_lt(tcalc_val a, tcalc_val b, int* out);
tcalc_err tcalc_val_lteq(tcalc_val a, tcalc_val b, int* out);
tcalc_err tcalc_val_gt(tcalc_val a, tcalc_val b, int* out);
tcalc_err tcalc_val_gteq(tcalc_val a, tcalc_val b, int* out);


/**
 * Unary functions:
*/
tcalc_err tcalc_val_ceil(tcalc_val a, double* out);
tcalc_err tcalc_val_floor(tcalc_val a, double* out);
tcalc_err tcalc_val_round(tcalc_val a, double* out);
tcalc_err tcalc_val_abs(tcalc_val a, double* out);


tcalc_err tcalc_val_sin(tcalc_val a, double* out);
tcalc_err tcalc_val_cos(tcalc_val a, double* out);
tcalc_err tcalc_val_tan(tcalc_val a, double* out);
tcalc_err tcalc_val_sec(tcalc_val a, double* out);
tcalc_err tcalc_val_csc(tcalc_val a, double* out);
tcalc_err tcalc_val_cot(tcalc_val a, double* out);

tcalc_err tcalc_val_asin(tcalc_val a, double* out);
tcalc_err tcalc_val_acos(tcalc_val a, double* out);
tcalc_err tcalc_val_atan(tcalc_val a, double* out);
tcalc_err tcalc_val_asec(tcalc_val a, double* out);
tcalc_err tcalc_val_acsc(tcalc_val a, double* out);
tcalc_err tcalc_val_acot(tcalc_val a, double* out);

tcalc_err tcalc_val_sinh(tcalc_val a, double* out);
tcalc_err tcalc_val_cosh(tcalc_val a, double* out);
tcalc_err tcalc_val_tanh(tcalc_val a, double* out);
tcalc_err tcalc_val_asinh(tcalc_val a, double* out);
tcalc_err tcalc_val_acosh(tcalc_val a, double* out);
tcalc_err tcalc_val_atanh(tcalc_val a, double* out);


tcalc_err tcalc_val_sin_deg(tcalc_val a, double* out);
tcalc_err tcalc_val_cos_deg(tcalc_val a, double* out);
tcalc_err tcalc_val_tan_deg(tcalc_val a, double* out);
tcalc_err tcalc_val_sec_deg(tcalc_val a, double* out);
tcalc_err tcalc_val_csc_deg(tcalc_val a, double* out);
tcalc_err tcalc_val_cot_deg(tcalc_val a, double* out);

tcalc_err tcalc_val_asin_deg(tcalc_val a, double* out);
tcalc_err tcalc_val_acos_deg(tcalc_val a, double* out);
tcalc_err tcalc_val_atan_deg(tcalc_val a, double* out);
tcalc_err tcalc_val_asec_deg(tcalc_val a, double* out);
tcalc_err tcalc_val_acsc_deg(tcalc_val a, double* out);
tcalc_err tcalc_val_acot_deg(tcalc_val a, double* out);

tcalc_err tcalc_val_sinh_deg(tcalc_val a, double* out);
tcalc_err tcalc_val_cosh_deg(tcalc_val a, double* out);
tcalc_err tcalc_val_tanh_deg(tcalc_val a, double* out);
tcalc_err tcalc_val_asinh_deg(tcalc_val a, double* out);
tcalc_err tcalc_val_acosh_deg(tcalc_val a, double* out);
tcalc_err tcalc_val_atanh_deg(tcalc_val a, double* out);


tcalc_err tcalc_val_unary_plus(tcalc_val a, double* out);
tcalc_err tcalc_val_unary_minus(tcalc_val a, double* out);
tcalc_err tcalc_val_log(tcalc_val a, double* out);
tcalc_err tcalc_val_sqrt(tcalc_val a, double* out);
tcalc_err tcalc_val_cbrt(tcalc_val a, double* out);

tcalc_err tcalc_val_ln(tcalc_val a, double* out);
tcalc_err tcalc_val_exp(tcalc_val a, double* out);

/**
 * Binary functions:
*/
tcalc_err tcalc_val_add(tcalc_val a, tcalc_val b, double* out);
tcalc_err tcalc_val_subtract(tcalc_val a, tcalc_val b, double* out);
tcalc_err tcalc_val_multiply(tcalc_val a, tcalc_val b, double* out);
tcalc_err tcalc_val_divide(tcalc_val a, tcalc_val b, double* out);
tcalc_err tcalc_val_mod(tcalc_val a, tcalc_val b, double* out);

tcalc_err tcalc_val_pow(tcalc_val a, tcalc_val b, double* out);
tcalc_err tcalc_val_atan2(tcalc_val a, tcalc_val b, double* out);
tcalc_err tcalc_val_atan2_deg(tcalc_val a, tcalc_val b, double* out);

#endif
