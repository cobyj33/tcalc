#include "tcalc_func.h"

#include "tcalc_error.h"

#include <math.h>
#include <errno.h>
#include <float.h>

tcalc_error_t tcalc_add(double a, double b, double* out) {
  if (DBL_MAX - b <= a) return TCALC_OVERFLOW;
  *out = a + b;
  return TCALC_OK;
}

tcalc_error_t tcalc_subtract(double a, double b, double* out) {
  if (DBL_MIN + b <= a) return TCALC_OVERFLOW;
  *out = a - b;
  return TCALC_OK;
}

tcalc_error_t tcalc_multiply(double a, double b, double* out) {
  if (b < 0) { // b is negative
    if (a <= DBL_MAX / b) return TCALC_OVERFLOW;
  } else { // b > 0
    if (a >= DBL_MAX / b) return TCALC_OVERFLOW;
  }

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

