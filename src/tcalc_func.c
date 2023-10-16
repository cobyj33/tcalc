#include "tcalc_func.h"

#include "tcalc_error.h"

#include <math.h>
#include <errno.h>
#include <float.h>

tcalc_error_t tcalc_unary_plus(double a, double* out) {
  *out = a;
  return TCALC_OK;
}

tcalc_error_t tcalc_unary_minus(double a, double* out) {
  *out = -a;
  return TCALC_OK;
}

tcalc_error_t tcalc_add(double a, double b, double* out) {
  // if (DBL_MAX - b <= a) return TCALC_OVERFLOW;
  *out = a + b;
  return TCALC_OK;
}

tcalc_error_t tcalc_subtract(double a, double b, double* out) {
  // if (DBL_MIN + b <= a) return TCALC_OVERFLOW;
  *out = a - b;
  return TCALC_OK;
}

tcalc_error_t tcalc_multiply(double a, double b, double* out) {
  // if (b < 0) { // b is negative
  //   if (a <= DBL_MAX / b) return TCALC_OVERFLOW;
  // } else { // b > 0
  //   if (a >= DBL_MAX / b) return TCALC_OVERFLOW;
  // }

  *out = a * b;
  return TCALC_OK;
}

tcalc_error_t tcalc_divide(double a, double b, double* out) {
  if (b == 0) return TCALC_DIVISION_BY_ZERO;
  // TODO: Possibly more overflow and underflow?

  *out =  a / b;
  return TCALC_OK;
}

tcalc_error_t tcalc_pow(double a, double b, double* out) {
  if (a == 0 && b == 0) return TCALC_NOT_IN_DOMAIN;
  if (a == 0 && b < 0) return TCALC_NOT_IN_DOMAIN;
  errno = 0;
  *out =  pow(a, b);

  if (math_errhandling & MATH_ERRNO && errno == ERANGE) return TCALC_OVERFLOW;
  if (math_errhandling & MATH_ERRNO && errno == EDOM) return TCALC_NOT_IN_DOMAIN;
  if (*out == HUGE_VAL) return TCALC_OVERFLOW;
  return TCALC_OK;
}

tcalc_error_t tcalc_ceil(double a, double* out) {
  *out = ceil(a);
  return TCALC_OK;
}

tcalc_error_t tcalc_floor(double a, double* out) {
  *out = floor(a);
  return TCALC_OK;
}

tcalc_error_t tcalc_round(double a, double* out) {
  *out = round(a);
  return TCALC_OK;
}

tcalc_error_t tcalc_mod(double a, double b, double* out) {
  if (b == 0) return TCALC_NOT_IN_DOMAIN;
  *out = fmod(a, b);
  return *out;
}

tcalc_error_t tcalc_abs(double a, double* out) {
  *out = fabs(a);
  return TCALC_OK;
}


tcalc_error_t tcalc_sin(double a, double* out) {
  *out = sin(a);
  return TCALC_OK;
}

tcalc_error_t tcalc_cos(double a, double* out) {
  *out = cos(a);
  return TCALC_OK;
}

tcalc_error_t tcalc_tan(double a, double* out) {

}

tcalc_error_t tcalc_asin(double a, double* out) {
  if (a < -1.0 || a > 1.0) return TCALC_NOT_IN_DOMAIN;

  return TCALC_OK;
}

tcalc_error_t tcalc_acos(double a, double* out) {
  if (a < -1.0 || a > 1.0) return TCALC_NOT_IN_DOMAIN;

  return TCALC_OK;
}

tcalc_error_t tcalc_atan(double a, double* out) {
  *out = atan(a);
  return TCALC_OK;
}


tcalc_error_t tcalc_sinh(double a, double* out) {


}

tcalc_error_t tcalc_cosh(double a, double* out) {

}

tcalc_error_t tcalc_tanh(double a, double* out) {

}

tcalc_error_t tcalc_asinh(double a, double* out) {

}

tcalc_error_t tcalc_acosh(double a, double* out) {

}

tcalc_error_t tcalc_atanh(double a, double* out) {

}

tcalc_error_t tcalc_log(double a, double* out) {
  if (a < 0) return TCALC_NOT_IN_DOMAIN;
  if (a == 0) return TCALC_OVERFLOW;
  
  *out = log(a) / log(10);
  return TCALC_OK;
}

tcalc_error_t tcalc_sqrt(double a, double* out) {
  if (a < 0) return TCALC_NOT_IN_DOMAIN;
  *out = sqrt(a);
  return TCALC_OK;
}


tcalc_error_t tcalc_cbrt(double a, double* out) {
  
}

tcalc_error_t tcalc_ln(double a, double* out) {
  if (a < 0) return TCALC_NOT_IN_DOMAIN;
  if (a == 0) return TCALC_OVERFLOW;

  *out = log(a);
  return TCALC_OK;
}

tcalc_error_t tcalc_exp(double a, double* out);
