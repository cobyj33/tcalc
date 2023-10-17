#ifndef TCALC_ERROR_H
#define TCALC_ERROR_H

/**
 * tcalc_error.h - Error handling in tcalc
 * 
 * Error handling will
 * 
 * In tcalc, functions that have any chance to fail should return a tcalc_error_t
 * enum. 
 * This results in a lot of *out parameters in the tcalc program, so I hope you
 * understand your pointers well.
 * 
 * If a function does not return tcalc_error_t, that MUST mean that it will
 * never fail. This even applies to searching for indexes. Please don't return -1
 * to mean not found, TCALC_NOT_FOUND exists.
*/

#define TCALC_ERROR_MAX_SIZE 512

/**
 * 
*/
void tcalc_getfullerror(char* out);

/**
 * 
*/
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

  TCALC_UNIMPLEMENTED = -12,

} tcalc_error_t;

const char* tcalc_strerrcode(tcalc_error_t err);

#endif