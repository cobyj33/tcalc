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
 * never fail. This applies to every function.
 * (*There is a gray area with functions that check for the existence of a value
 * or condition, as they don't technically "fail" in the
 * sense that they always return a valid boolean value. In this case, they can return int)
 * 
 * As an extra note, if the function contains any type of allocation,
 * it should return tcalc_err, as an allocation failure is always possible.
 * 
 * The global error string is generally only set when actual lexical, syntactical, or semantical
 * errors are encountered. This is so code isn't cluttered with reporting error strings
 * for error code's like TCALC_ERR_BAD_ALLOC or TCALC_ERR_OUT_OF_BOUNDS, which generally
 * are more so exceptional than actual errors, as they are
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
 * @brief A general error type for operations in tcalc.
 * 
 * A note on some error types:
 * 
 * TCALC_ERR_NOT_FOUND - This should be returned from getter functions when a
 * value is not found. This shouldn't be returned from "contains" or "has" type
 * functions when they don't find anything, as a "contains" or "has" function
 * not finding a value is not an error, but a valid return type of false.
 * 
 * TCALC_ERR_UNKNOWN - This should only be returned when there is no other way
 * for the programmer to know what error happened, such as a code block which
 * was supposed to be unreachable.
 * 
 * 
 * 
 * When adding new errors, make sure to adjust returned strings in
 * tcalc_strerrcode
*/
typedef enum tcalc_err {
  TCALC_ERR_OK = 0, // important that this stays 0
  TCALC_ERR_OUT_OF_BOUNDS = -43110,
  TCALC_ERR_BAD_ALLOC,
  TCALC_ERR_INVALID_ARG,
  TCALC_ERR_INVALID_OP,
  TCALC_ERR_OVERFLOW,
  TCALC_ERR_UNDERFLOW,
  TCALC_ERR_STOP_ITER,
  TCALC_ERR_NOT_FOUND,
  TCALC_ERR_DIVISION_BY_ZERO,
  TCALC_ERR_NOT_IN_DOMAIN,
  TCALC_ERR_UNKNOWN_IDENTIFIER,
  TCALC_ERR_UNBALANCED_GROUPING_SYMBOLS,
  TCALC_ERR_UNKNOWN_TOKEN,
  TCALC_ERR_WRONG_ARITY,
  TCALC_ERR_UNCLOSED_FUNC,
  TCALC_ERR_UNCALLED_FUNC,
  TCALC_ERR_MALFORMED_BINEXP,
  TCALC_ERR_MALFORMED_INPUT,

  // add new errors above this
  TCALC_ERR_UNIMPLEMENTED,
  TCALC_ERR_UNKNOWN // TCALC_ERR_UNKNOWN MUST BE THE LAST MEMBER
} tcalc_err;

/**
 * A macro which sets the err variable to what expr evaluates to, and also
 * evaluates to true if the expr returns an error code that is NOT TCALC_ERR_OK.
 * 
 * @param err a tcalc_err variable which will be set to what expr evaluates to
 * @param expr an expression which evaluates to a tcalc_err value.
*/
#define tc_failed(err, expr) ((err) = (expr)) != TCALC_ERR_OK

/**
 * ret_on_err evaluates expr, sets err to equal what expression evaluates to,
 * and returns err from whatever function ret_on_err is used in if err is not TCALC_ERR_OK
 * 
 * @param err a tcalc_err variable which will be set to what expr evaluates to
 * @param expr an expression which **evaluates** to a tcalc_err value.
*/
#define ret_on_err(err, expr) if (tc_failed(err, expr)) return err


#define reterr_on_true(err, expr, erronerr) if (expr) { err = (erronerr); return err; }

/**
 * tcalc has a general pattern of a cleanup: label at the bottom of functions
 * which have to do some sort of cleaning up in case of errors, such as freeing
 * memory. Since cleaning up is so common, the pattern of setting a "cleanup:"
 * label at the end of the function and jumping to it on errors is also quite common,
 * and gives birth to macros like cleanup_on_err which reduce the boilerplate for
 * this pattern.
*/

/**
 * cleanup_on_err evaluates expr, sets err to equal what expression evaluates to,
 * and jumps to a label named "cleanup" if err is not TCALC_ERR_OK. Note that this
 * of course means that any function cleanup_on_err is used, there should be a
 * label named "cleanup" that cleanup_on_err can actually jump to. 
 * 
 * cleanup_on_err is equivalent to ret_on_err, except that cleanup_on_err jumps
 * to a label named "cleanup" rather than returning err directly.
 * 
 * @param err a tcalc_err variable which will be set to what expr evaluates to
 * @param expr an expression which evaluates to a tcalc_err value.
 */
#define cleanup_on_err(err, expr) if (tc_failed(err, expr)) goto cleanup

/**
 * cleanup_on_macerr is similar to cleanup_on_err, but is intended to be used
 * with macros (or statements) which will set err rather than general expressions.
 * Note that the macro or statement given should not evaluate to a tcalc_err
 * value, but should actually mutate the given value of err.
 * 
 * @param err a tcalc_err variable which will be tested after mac runs to jump
 * to a label named "cleanup"
 * @param mac a macro or statement which will be run before err is tested. mac
 * should mutate the given err value directly.
*/
#define cleanup_on_macerr(err, mac) do { mac; if (err) goto cleanup; } while (0)

/**
 * A shorthand to evaluate to either an error or TCALC_ERR_OK depending on whether
 * the given expression evaluates to true or false. 
 * 
 * @param expr the expression to evaluate. Should evaluate to a truthy or falsey value
 * @param erronerr the tcalc_err for err_pred to evaluate to **if expr evaluates to true**
*/
#define err_pred(expr, erronerr) expr ? erronerr : TCALC_ERR_OK

#define cleanup_if(err, expr, erronerr) cleanup_on_err(err, err_pred(expr, erronerr))

/**
 * Return a string representation of a tcalc_err error code
 * 
 * The string returned by tcalc_strerrcode does NOT have to be freed,
 * as it will always be a string literal.
 * 
 * @param err The tcalc_err err code which will be turned
 * into a string representation
*/
const char* tcalc_strerrcode(tcalc_err err);

#endif