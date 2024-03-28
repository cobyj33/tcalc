#ifndef TCALC_ERROR_H
#define TCALC_ERROR_H


/**
 * tcalc_error.h - Error handling in tcalc
 * 
 * In tcalc, functions that have any chance to fail should return a tcalc_err
 * enum. As a result, many functions really return their data to an *out pointer that
 * the user must supply. Usually, functions will not check if this is NULL or not, and
 * passing a NULL to an *out parameter or an *out_size parameter will be undefined unless
 * otherwise stated so by the given function's documentation
 * 
 * If a function does not return tcalc_err, that MUST mean that it will
 * never fail. This applies to EVERYTHING.
 * (There is a gray area with functions that check for the existence of a value
 * or condition, as they don't technically "fail" in the
 * sense that they always return a valid boolean value. In this case, they can return int)
 * 
 * 
 * As an extra note, if the function contains any type of allocation,
 * it should return tcalc_err, as an allocation failure is always possible.
 * 
 * The global error is generally only set when actual lexical, syntactical, or semantical
 * errors are encountered. This is so code isn't cluttered with reporting error strings
 * for error code's like TCALC_BAD_ALLOC or TCALC_OUT_OF_BOUNDS, which generally
 * are more so exceptional than actual errors, as they can't be handled and are
 * largely independent of invalid input or computational failures. 
*/

#define TCALC_ERROR_MAX_SIZE 512

/**
 * Get the globally set tcalc error
 * 
 * The buffer "out" should have a size at least equal to the macro TCALC_ERROR_MAX_SIZE
*/
void tcalc_getfullerror(char* out);

/**
 * Errors don't have to be set by every error or exceptional encounter in tcalc,
 * and setting error strings should not replace returning error codes.
 * 
 * If the input string is longer than TCALC_ERROR_MAX_SIZE,
 * it will be truncated to TCALC_ERROR_MAX_SIZE.
*/
void tcalc_setfullerror(const char* error);

/**
 * Formatted version of tcalc_setfullerror
 * 
 * Formatting is exactly the same as standard stdio functions like printf
 * 
 * If the expanded formatted string is longer than TCALC_ERROR_MAX_SIZE,
 * it will be truncated to TCALC_ERROR_MAX_SIZE.
*/
void tcalc_setfullerrorf(const char* format, ...);

/**
 * A note on some error types:
 * 
 * TCALC_NOT_FOUND - This should be returned from getter functions when a
 * value is not found. This shouldn't be returned from "contains" or "has" type
 * functions when they don't find anything, as a "contains" or "has" function
 * not finding a value is not an error, but a valid return type of false.
 * 
 * TCALC_UNKNOWN - This should only be returned when there is no other way
 * for the programmer to know what error happened, such as a code block which
 * was supposed to be unreachable.
*/
typedef enum tcalc_err {
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
  TCALC_UNKNOWN_TOKEN,
  TCALC_UNIMPLEMENTED,
  TCALC_UNKNOWN
} tcalc_err;

#define tc_failed(err, expr) ((err) = (expr)) != TCALC_OK
#define cleanup_on_err(err, expr) if (tc_failed(err, expr)) goto cleanup; 

/**
 * The string returned by tcalc_strerrcode does not have to be freed,
 * as it will always be a string literal.
*/
const char* tcalc_strerrcode(tcalc_err err);

#endif