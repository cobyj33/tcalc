#include "tcalc.h"

#include <stdbool.h>
#include <math.h>
#include <errno.h>
#include <float.h>

// TODO: Handling overflow and underflow?

/**
 * Logical Functions
*/
bool tcalc_not(bool a) {
  return !a;
}

bool tcalc_and(bool a, bool b) {
  return a && b;
}

bool tcalc_or(bool a, bool b) {
  return a || b;
}

bool tcalc_nand(bool a, bool b) {
  return !(a && b);
}

bool tcalc_nor(bool a, bool b) {
  return !(a || b);
}

bool tcalc_xor(bool a, bool b) {
  return (a && !b) || (!a && b);
}

bool tcalc_xnor(bool a, bool b) {
  // note that xnor and equals are literally the same thing
  return (a && b) || (!a && !b);
}

bool tcalc_matcond(bool a, bool b) {
  return !a || b;
}

/**
 * We can't just say if they are directly equal as ```a == b```, since
 * a truthy value in C simply means that the value is not 0.
*/
bool tcalc_equals_l(bool a, bool b) {
  return (a && b) || (!a && !b);
}

bool tcalc_nequals_l(bool a, bool b) {
  return (a && !b) || (!a && b);
}


/**
 * Relational Operators
*/
bool tcalc_equals(double a, double b) {
  return fabs(a - b) < 1e-9;
}

bool tcalc_nequals(double a, double b) {
  return !tcalc_equals(a, b);
}

bool tcalc_lt(double a, double b) {
  return a < b && !tcalc_equals(a, b);
}

bool tcalc_lteq(double a, double b) {
  return a < b || tcalc_equals(a, b);
}

bool tcalc_gt(double a, double b) {
  return a > b && !tcalc_equals(a, b);
}

bool tcalc_gteq(double a, double b) {
  return a > b || tcalc_equals(a, b);
}

tcalc_err tcalc_unary_plus(double a, double* out) {
  *out = a;
  return TCALC_ERR_OK;
}

tcalc_err tcalc_unary_minus(double a, double* out) {
  *out = -a;
  return TCALC_ERR_OK;
}

tcalc_err tcalc_add(double a, double b, double* out) {
  *out = a + b;
  return TCALC_ERR_OK;
}

tcalc_err tcalc_subtract(double a, double b, double* out) {
  *out = a - b;
  return TCALC_ERR_OK;
}

tcalc_err tcalc_multiply(double a, double b, double* out) {
  *out = a * b;
  return TCALC_ERR_OK;
}

tcalc_err tcalc_divide(double a, double b, double* out) {
  if (tcalc_equals(b, 0)) return TCALC_ERR_DIV_BY_ZERO;

  *out =  a / b;
  return TCALC_ERR_OK;
}

tcalc_err tcalc_pow(double a, double b, double* out) {
  if (tcalc_equals(a, 0) && tcalc_equals(b, 0)) return TCALC_ERR_NOT_IN_DOMAIN;
  if (tcalc_equals(a, 0) && tcalc_lt(b, 0)) return TCALC_ERR_NOT_IN_DOMAIN;
  errno = 0;
  *out =  pow(a, b);

  if (math_errhandling & MATH_ERRNO && errno == ERANGE) return TCALC_ERR_OVERFLOW;
  if (math_errhandling & MATH_ERRNO && errno == EDOM) return TCALC_ERR_NOT_IN_DOMAIN;
  if (*out == HUGE_VAL) return TCALC_ERR_OVERFLOW;
  return TCALC_ERR_OK;
}

tcalc_err tcalc_ceil(double a, double* out) {
  *out = ceil(a);
  return TCALC_ERR_OK;
}

tcalc_err tcalc_floor(double a, double* out) {
  *out = floor(a);
  return TCALC_ERR_OK;
}

tcalc_err tcalc_round(double a, double* out) {
  *out = round(a);
  return TCALC_ERR_OK;
}

tcalc_err tcalc_mod(double a, double b, double* out) {
  if (tcalc_equals(b, 0)) return TCALC_ERR_NOT_IN_DOMAIN;
  *out = fmod(a, b);
  return TCALC_ERR_OK;
}

tcalc_err tcalc_abs(double a, double* out) {
  *out = fabs(a);
  return TCALC_ERR_OK;
}

#define TCALC_TRIG_DEG_UFUNCDEF(radfuncname, degfuncname) \
  tcalc_err degfuncname(double a, double* out) { \
    return radfuncname(a * TCALC_DEG_TO_RAD, out); \
  }

#define TCALC_TRIG_DEG_BINFUNCDEF(radfuncname, degfuncname) \
  tcalc_err degfuncname(double a, double b, double* out) { \
    return radfuncname(a * TCALC_DEG_TO_RAD, b * TCALC_DEG_TO_RAD, out); \
  }

tcalc_err tcalc_sin(double a, double* out) {
  *out = sin(a);
  return TCALC_ERR_OK;
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_sin, tcalc_sin_deg)

tcalc_err tcalc_cos(double a, double* out) {
  *out = cos(a);
  return TCALC_ERR_OK;
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_cos, tcalc_cos_deg)

tcalc_err tcalc_tan(double a, double* out) {
  if (tcalc_equals(fmod(a - TCALC_PI / 2, TCALC_PI), 0.0)) return TCALC_ERR_OVERFLOW;
  *out = tan(a);
  return TCALC_ERR_OK;
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_tan, tcalc_tan_deg)

tcalc_err tcalc_sec(double a, double* out) {
  tcalc_err err = TCALC_ERR_OK;
  double reciprocal_res;
  if ((err == tcalc_cos(a, &reciprocal_res)) != TCALC_ERR_OK) return err;
  err = tcalc_divide(1.0, reciprocal_res, out);
  if (err == TCALC_ERR_DIV_BY_ZERO) return TCALC_ERR_NOT_IN_DOMAIN;
  return err;
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_sec, tcalc_sec_deg)

tcalc_err tcalc_csc(double a, double* out) {
  tcalc_err err = TCALC_ERR_OK;
  double reciprocal_res;
  if ((err == tcalc_sin(a, &reciprocal_res)) != TCALC_ERR_OK) return err;
  err = tcalc_divide(1.0, reciprocal_res, out);
  if (err == TCALC_ERR_DIV_BY_ZERO) return TCALC_ERR_NOT_IN_DOMAIN;
  return err;
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_csc, tcalc_csc_deg)

tcalc_err tcalc_cot(double a, double* out) {
  tcalc_err err = TCALC_ERR_OK;
  double reciprocal_res;
  if ((err == tcalc_tan(a, &reciprocal_res)) != TCALC_ERR_OK) return err;
  err = tcalc_divide(1.0, reciprocal_res, out);
  if (err == TCALC_ERR_DIV_BY_ZERO) return TCALC_ERR_NOT_IN_DOMAIN;
  return err;
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_cot, tcalc_cot_deg)

tcalc_err tcalc_asin(double a, double* out) {
  if (tcalc_lt(a, -1.0) || tcalc_gt(a, 1.0)) return TCALC_ERR_NOT_IN_DOMAIN;
  *out = asin(a);
  return TCALC_ERR_OK;
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_asin, tcalc_asin_deg)

tcalc_err tcalc_acos(double a, double* out) {
  if (tcalc_lt(a, -1.0) || tcalc_gt(a, 1.0)) return TCALC_ERR_NOT_IN_DOMAIN;
  *out = acos(a);
  return TCALC_ERR_OK;
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_acos, tcalc_acos_deg)

tcalc_err tcalc_atan(double a, double* out) {
  *out = atan(a);
  return TCALC_ERR_OK;
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_atan, tcalc_atan_deg)

tcalc_err tcalc_atan2(double a, double b, double* out) {
  if (tcalc_equals(a, 0.0) && tcalc_equals(b, 0.0)) return TCALC_ERR_NOT_IN_DOMAIN;
  *out = atan2(a, b);
  return TCALC_ERR_OK;
}
TCALC_TRIG_DEG_BINFUNCDEF(tcalc_atan2, tcalc_atan2_deg)

tcalc_err tcalc_asec(double a, double* out) {
  if (tcalc_equals(a, 0.0)) return TCALC_ERR_NOT_IN_DOMAIN;
  return tcalc_acos(1/a, out);
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_asec, tcalc_asec_deg)

tcalc_err tcalc_acsc(double a, double* out) {
  if (tcalc_equals(a, 0.0)) return TCALC_ERR_NOT_IN_DOMAIN;
  return tcalc_asin(1/a, out);
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_acsc, tcalc_acsc_deg)

tcalc_err tcalc_acot(double a, double* out) {
  if (tcalc_equals(a, 0.0)) {
    *out = 0.0;
    return TCALC_ERR_OK;
  }

  if (a > 0.0) {
    return tcalc_atan(1/a, out);
  }
  return tcalc_atan(1/a + TCALC_PI, out);
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_acot, tcalc_acot_deg)

tcalc_err tcalc_sinh(double a, double* out) {
  errno = 0;
  *out =  sinh(a);

  if (math_errhandling & MATH_ERRNO && errno == ERANGE) return TCALC_ERR_OVERFLOW;
  if (*out == HUGE_VAL) return TCALC_ERR_OVERFLOW;
  return TCALC_ERR_OK;
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_sinh, tcalc_sinh_deg)

tcalc_err tcalc_cosh(double a, double* out) {
  errno = 0;
  *out =  cosh(a);

  if (math_errhandling & MATH_ERRNO && errno == ERANGE) return TCALC_ERR_OVERFLOW;
  if (*out == HUGE_VAL) return TCALC_ERR_OVERFLOW;
  return TCALC_ERR_OK;
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_cosh, tcalc_cosh_deg)

tcalc_err tcalc_tanh(double a, double* out) {
  *out = tanh(a);
  return TCALC_ERR_OK;
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_tanh, tcalc_tanh_deg)

tcalc_err tcalc_asinh(double a, double* out) {
  *out = asinh(a);
  return TCALC_ERR_OK;
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_asinh, tcalc_asinh_deg)

tcalc_err tcalc_acosh(double a, double* out) {
  if (tcalc_lt(a, 1.0)) return TCALC_ERR_NOT_IN_DOMAIN;
  *out = acosh(a);
  return TCALC_ERR_OK;
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_acosh, tcalc_acosh_deg)

tcalc_err tcalc_atanh(double a, double* out) {
  if (tcalc_lt(a, -1.0) || tcalc_gt(a, 1.0)) return TCALC_ERR_NOT_IN_DOMAIN;
  if (tcalc_equals(a, -1.0) || tcalc_equals(a, 1.0)) return TCALC_ERR_OVERFLOW;
  *out = atanh(a);
  return TCALC_ERR_OK;
}
TCALC_TRIG_DEG_UFUNCDEF(tcalc_atanh, tcalc_atanh_deg)

tcalc_err tcalc_log(double a, double* out) {
  if (tcalc_lt(a, 0.0)) return TCALC_ERR_NOT_IN_DOMAIN;
  if (tcalc_equals(a, 0.0)) return TCALC_ERR_OVERFLOW;

  *out = log(a) / TCALC_LN10;
  return TCALC_ERR_OK;
}

tcalc_err tcalc_sqrt(double a, double* out) {
  if (tcalc_lt(a, 0.0)) return TCALC_ERR_NOT_IN_DOMAIN;
  *out = sqrt(a);
  return TCALC_ERR_OK;
}

tcalc_err tcalc_cbrt(double a, double* out) {
  *out = cbrt(a);
  return TCALC_ERR_OK;
}

tcalc_err tcalc_ln(double a, double* out) {
  if (tcalc_lt(a, 0.0) || tcalc_equals(a, 0.0)) return TCALC_ERR_NOT_IN_DOMAIN;

  *out = log(a);
  return TCALC_ERR_OK;
}

tcalc_err tcalc_exp(double a, double* out) {
  *out = exp(a);

  if (math_errhandling & MATH_ERRNO && errno == ERANGE) return TCALC_ERR_OVERFLOW;
  if (*out == HUGE_VAL) return TCALC_ERR_OVERFLOW;
  return TCALC_ERR_OK;
}
