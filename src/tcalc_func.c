#include "tcalc_func.h"

#include "tcalc_error.h"
#include "tcalc_constants.h"

#include <math.h>
#include <errno.h>
#include <float.h>

int tcalc_equals(double a, double b) {
  return fabs(a - b) < 1e-9;
}

int tcalc_lt(double a, double b) {
  return a < b && !tcalc_equals(a, b);
}

int tcalc_lteq(double a, double b) {
  return a < b || tcalc_equals(a, b);
}

int tcalc_gt(double a, double b) {
  return a > b && !tcalc_equals(a, b);
}

int tcalc_gteq(double a, double b) {
  return a > b || tcalc_equals(a, b);
}

tcalc_err tcalc_unary_plus(double a, double* out) {
  *out = a;
  return TCALC_OK;
}

tcalc_err tcalc_unary_minus(double a, double* out) {
  *out = -a;
  return TCALC_OK;
}

tcalc_err tcalc_add(double a, double b, double* out) {
  // if (DBL_MAX - b <= a) return TCALC_OVERFLOW;

  *out = a + b;
  return TCALC_OK;
}

tcalc_err tcalc_subtract(double a, double b, double* out) {
  // if (DBL_MIN + b <= a) return TCALC_OVERFLOW;
  *out = a - b;
  return TCALC_OK;
}

tcalc_err tcalc_multiply(double a, double b, double* out) {
  // if (b < 0) { // b is negative
  //   if (a <= DBL_MAX / b) return TCALC_OVERFLOW;
  // } else { // b > 0
  //   if (a >= DBL_MAX / b) return TCALC_OVERFLOW;
  // }

  *out = a * b;
  return TCALC_OK;
}

tcalc_err tcalc_divide(double a, double b, double* out) {
  if (tcalc_equals(b, 0)) return TCALC_DIVISION_BY_ZERO;
  // TODO: Possibly more overflow and underflow?

  *out =  a / b;
  return TCALC_OK;
}

tcalc_err tcalc_pow(double a, double b, double* out) {
  if (tcalc_equals(a, 0) && tcalc_equals(b, 0)) return TCALC_NOT_IN_DOMAIN;
  if (tcalc_equals(a, 0) && tcalc_lt(b, 0)) return TCALC_NOT_IN_DOMAIN;
  errno = 0;
  *out =  pow(a, b);

  if (math_errhandling & MATH_ERRNO && errno == ERANGE) return TCALC_OVERFLOW;
  if (math_errhandling & MATH_ERRNO && errno == EDOM) return TCALC_NOT_IN_DOMAIN;
  if (*out == HUGE_VAL) return TCALC_OVERFLOW;
  return TCALC_OK;
}

tcalc_err tcalc_ceil(double a, double* out) {
  *out = ceil(a);
  return TCALC_OK;
}

tcalc_err tcalc_floor(double a, double* out) {
  *out = floor(a);
  return TCALC_OK;
}

tcalc_err tcalc_round(double a, double* out) {
  *out = round(a);
  return TCALC_OK;
}

tcalc_err tcalc_mod(double a, double b, double* out) {
  if (tcalc_equals(b, 0)) return TCALC_NOT_IN_DOMAIN;
  *out = fmod(a, b);
  return TCALC_OK;
}

tcalc_err tcalc_abs(double a, double* out) {
  *out = fabs(a);
  return TCALC_OK;
}

tcalc_err tcalc_sin(double a, double* out) {
  *out = sin(a);
  return TCALC_OK;
}

tcalc_err tcalc_cos(double a, double* out) {
  *out = cos(a);
  return TCALC_OK;
}

tcalc_err tcalc_tan(double a, double* out) {
  if (tcalc_equals(fmod(a - M_PI / 2, M_PI), 0.0)) return TCALC_OVERFLOW;
  *out = tan(a);
  return TCALC_OK;
}

tcalc_err tcalc_sec(double a, double* out) {
  tcalc_err err = TCALC_OK;
  double reciprocal_res;
  if ((err == tcalc_cos(a, &reciprocal_res)) != TCALC_OK) return err;
  return tcalc_divide(1.0, reciprocal_res, out);
}

tcalc_err tcalc_csc(double a, double* out) {
  tcalc_err err = TCALC_OK;
  double reciprocal_res;
  if ((err == tcalc_sin(a, &reciprocal_res)) != TCALC_OK) return err;
  return tcalc_divide(1.0, reciprocal_res, out);
}

tcalc_err tcalc_cot(double a, double* out) {
  tcalc_err err = TCALC_OK;
  double reciprocal_res;
  if ((err == tcalc_tan(a, &reciprocal_res)) != TCALC_OK) return err;
  return tcalc_divide(1.0, reciprocal_res, out);
}

tcalc_err tcalc_asin(double a, double* out) {
  if (tcalc_lt(a, -1.0) || tcalc_gt(a, 1.0)) return TCALC_NOT_IN_DOMAIN;

  *out = asin(a);
  return TCALC_OK;
}

tcalc_err tcalc_acos(double a, double* out) {
  if (tcalc_lt(a, -1.0) || tcalc_gt(a, 1.0)) return TCALC_NOT_IN_DOMAIN;

  *out = acos(a);
  return TCALC_OK;
}

tcalc_err tcalc_atan(double a, double* out) {
  *out = atan(a);
  return TCALC_OK;
}

tcalc_err tcalc_asec(double a, double* out) {
  if (tcalc_equals(a, 0.0)) return TCALC_NOT_IN_DOMAIN;
  return tcalc_acos(1/a, out);
}

tcalc_err tcalc_acsc(double a, double* out) {
  if (tcalc_equals(a, 0.0)) return TCALC_NOT_IN_DOMAIN;
  return tcalc_asin(1/a, out);
}

tcalc_err tcalc_acot(double a, double* out) {
  if (tcalc_equals(a, 0.0)) {
    *out = 0.0;
    return TCALC_OK;
  }

  if (a > 0.0) {
    return tcalc_atan(1/a, out);
  }
  return tcalc_atan(1/a + M_PI, out);
}

tcalc_err tcalc_sinh(double a, double* out) {
  errno = 0;
  *out =  sinh(a);

  if (math_errhandling & MATH_ERRNO && errno == ERANGE) return TCALC_OVERFLOW;
  if (*out == HUGE_VAL) return TCALC_OVERFLOW;
  return TCALC_OK;
}

tcalc_err tcalc_cosh(double a, double* out) {
  errno = 0;
  *out =  cosh(a);

  if (math_errhandling & MATH_ERRNO && errno == ERANGE) return TCALC_OVERFLOW;
  if (*out == HUGE_VAL) return TCALC_OVERFLOW;
  return TCALC_OK;
}

tcalc_err tcalc_tanh(double a, double* out) {
  *out = tanh(a);
  return TCALC_OK;
}

tcalc_err tcalc_asinh(double a, double* out) {
  *out = asinh(a);
  return TCALC_OK;
}

tcalc_err tcalc_acosh(double a, double* out) {
  if (tcalc_lt(a, 1.0)) return TCALC_NOT_IN_DOMAIN;
  *out = acosh(a);
  return TCALC_OK;
}

tcalc_err tcalc_atanh(double a, double* out) {
  if (tcalc_lt(a, -1.0) || tcalc_gt(a, 1.0)) return TCALC_NOT_IN_DOMAIN;
  if (tcalc_equals(a, -1.0) || tcalc_equals(a, 1.0)) return TCALC_OVERFLOW;
  *out = atanh(a);
  return TCALC_OK;
}

tcalc_err tcalc_log(double a, double* out) {
  if (tcalc_lt(a, 0.0)) return TCALC_NOT_IN_DOMAIN;
  if (tcalc_equals(a, 0.0)) return TCALC_OVERFLOW;
  
  *out = log(a) / M_LN10;
  return TCALC_OK;
}

tcalc_err tcalc_sqrt(double a, double* out) {
  if (tcalc_lt(a, 0.0)) return TCALC_NOT_IN_DOMAIN;
  *out = sqrt(a);
  return TCALC_OK;
}

tcalc_err tcalc_cbrt(double a, double* out) {
  *out = cbrt(a);
  return TCALC_OK;
}

tcalc_err tcalc_ln(double a, double* out) {
  if (tcalc_lt(a, 0.0) || tcalc_equals(a, 0.0)) return TCALC_NOT_IN_DOMAIN;

  *out = log(a);
  return TCALC_OK;
}

tcalc_err tcalc_exp(double a, double* out) {
  *out = exp(a);

  if (math_errhandling & MATH_ERRNO && errno == ERANGE) return TCALC_OVERFLOW;
  if (*out == HUGE_VAL) return TCALC_OVERFLOW;
  return TCALC_OK;
}