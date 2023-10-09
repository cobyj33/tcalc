#ifndef TCALC_ERROR_H
#define TCALC_ERROR_H

#define TCALC_ERROR_MAX_SIZE 512

void tcalc_getfullerror(char* out);
void tcalc_setfullerror(const char* error);

typedef enum {
  TCALC_OK = 0,
  TCALC_OUT_OF_BOUNDS = -1,
  TCALC_BAD_ALLOC = -2,
  TCALC_INVALID_ARG = -3,
  TCALC_INVALID_OP = -4,
  TCALC_OVERFLOW = -5,
  TCALC_UNDERFLOW = -6,
  TCALC_STOP_ITER = -7,
  TCALC_NOT_FOUND = -8,
  TCALC_DIVISION_BY_ZERO = -9,
  TCALC_NOT_IN_DOMAIN = -10,

  TCALC_UNBALANCED_GROUPING_SYMBOLS = -11,


} tcalc_error_t;

const char* tcalc_strerrcode(tcalc_error_t err);

#endif