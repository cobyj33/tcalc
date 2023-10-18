#ifndef TCALC_ERROR_H
#define TCALC_ERROR_H


/**
 * tcalc_error.h - Error handling in tcalc
 * 
 * In tcalc, functions that have any chance to fail should return a tcalc_error_t
 * enum. 
 * This results in a lot of *out parameters in the tcalc program, so I hope you
 * understand your pointers well.
 * 
 * If a function does not return tcalc_error_t, that MUST mean that it will
 * never fail. This applies to EVERYTHING. If the function contains any type of
 * allocation, it should return tcalc_error_t, as an allocation failure is always possible.
 * 
 * 
 * As of right now, the error string setting and get functions are not really used
 * because of constant development. Ideally though, any time an error is thrown,
 * a string will be written that can be obtained with tcalc_getfullerror
*/

#define TCALC_ERROR_MAX_SIZE 512

/**
 * The buffer "out" should have a size at least equal to the macro TCALC_ERROR_MAX_SIZE
*/
void tcalc_getfullerror(char* out);

/**
 * 
 * Generally, 
 * 
 * If the given error
*/
void tcalc_setfullerror(const char* error);

/**
 * Formatted version of tcalc_setfullerror
 * 
 * Formatting is exactly the same as standard stdio functions like printf
*/
void tcalc_setfullerrorf(const char* format, ...);

typedef enum tcalc_error_t {
  TCALC_OK = 0,
  TCALC_OUT_OF_BOUNDS = -43110,
  TCALC_BAD_ALLOC,
  TCALC_INVALID_ARG,
  TCALC_INVALID_OP,
  TCALC_OVERFLOW,
  TCALC_UNDERFLOW,
  TCALC_STOP_ITER,
  TCALC_NOT_FOUND,
  TCALC_DIVISION_BY_ZERO,
  TCALC_NOT_IN_DOMAIN,
  TCALC_UNKNOWN_IDENTIFIER,
  TCALC_UNBALANCED_GROUPING_SYMBOLS,
  TCALC_UNIMPLEMENTED,
  TCALC_UNKNOWN
} tcalc_error_t;

const char* tcalc_strerrcode(tcalc_error_t err);

#endif