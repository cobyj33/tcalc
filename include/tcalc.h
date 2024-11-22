#ifndef TCALC_H
#define TCALC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define TCALC_PI 3.14159265358979323846
#define TCALC_E 2.7182818284590452354
#define TCALC_LN10 2.30258509299404568402
#define TCALC_RAD_TO_DEG (180.0 / TCALC_PI)
#define TCALC_DEG_TO_RAD (TCALC_PI / 180.0)

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
  #define TCALC_FORMAT_ATTRIB(format_type, format_param_i, vararg_start_i) \
    __attribute__((format (format_type, format_param_i, vararg_start_i)))
#else
  #define TCALC_FORMAT_ATTRIB(format_type, format_param_i, vararg_start_i)
#endif

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

/**
 * Quickly terminate the program with a printf-formatted message toward stderr.
 *
 * @param err a printf-formatted string which will be printed to stderr
 * @param ... printf-style varargs
*/
void tcalc_die(const char* err, ...);

void tcalc_errstkclear();
int tcalc_errstksize();
int tcalc_errstkpop();

bool tcalc_errstkadd(const char* funcname, const char* errstr);
bool tcalc_errstkaddf(const char* funcname, const char* format, ...) TCALC_FORMAT_ATTRIB(printf, 2, 3);

size_t tcalc_errstkpeek(char* out, size_t dsize);

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
  TCALC_ERR_DIV_BY_ZERO,
  TCALC_ERR_NOT_IN_DOMAIN,
  TCALC_ERR_UNKNOWN_ID,
  TCALC_ERR_UNBAL_GRPSYMS,
  TCALC_ERR_UNKNOWN_TOKEN,
  TCALC_ERR_WRONG_ARITY,
  TCALC_ERR_UNCLOSED_FUNC,
  TCALC_ERR_UNCALLED_FUNC,
  TCALC_ERR_MALFORMED_BINEXP,
  TCALC_ERR_MALFORMED_UNEXP,
  TCALC_ERR_MALFORMED_INPUT,
  TCALC_ERR_BAD_CAST,
  TCALC_ERR_UNPROCESSED_INPUT,

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
#define tc_failed(err, expr) (((err) = (expr)) != TCALC_ERR_OK)

/**
 * ret_on_err evaluates expr, sets err to equal what expression evaluates to,
 * and returns err from whatever function ret_on_err is used in if err is not TCALC_ERR_OK
 *
 * @param err a tcalc_err variable which will be set to what expr evaluates to
 * @param expr an expression which **evaluates** to a tcalc_err value.
*/
#define ret_on_err(err, expr) if (tc_failed(err, expr)) return (err)

#define ret_on_macerr(err, mac) do { mac; if (err) return err; } while (0)


#define reterr_on_true(err, expr, err_on_true) if (expr) { (err) = (err_on_true); return (err); }

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
 * @param err_on_true the tcalc_err for err_pred to evaluate to **if expr evaluates to true**
*/
#define err_pred(expr, err_on_true) ((expr) ? (err_on_true) : TCALC_ERR_OK)

#define cleanup_if(err, expr, err_on_true) cleanup_on_err(err, err_pred(expr, err_on_true))

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


/**
 * Notes about memory:
 *
 * All freeing functions should be defined to take a NULL pointer as a valid
 * argument. Freeing functions should never fail and therefore should never
 * return a tcalc_err value. This greatly simplifies code using any freeing
 * function since there is no null-checks that have to be used by other code
 * before using freeing functions, and the cleanup: label pattern can work
 * from anywhere within the function after allocated or soon-to-be-allocated
 * variables have been defined.
 *
 * Most special types have specific alloc and free functions. For simplicity,
 * and at least as of now, all types which have specific alloc and free functions
 * should only be constructed using these functions.
*/

#define TCALC_ALLOC_NR(x) (((x / 2) + 8) * 3)
#define TCALC_ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
 * Extended malloc function which aborts the program on malloc failure
 *
 * Note that this may not be the best way to handle malloc failures in general
 * code, as interactive sessions could terminate unexpectedly which may have
 * been truly recoverable with more graceful methods.
 *
 * @param size the size in bytes that the allocated memory block should be
 * @returns A guaranteed non-NULL pointer to the allocated memory block
*/
void* tcalc_xmalloc(size_t);

/**
 * Extended calloc function which aborts the program on malloc failure
 *
 * Note that this may not be the best way to handle malloc failures in general
 * code, as interactive sessions could terminate unexpectedly which may have
 * been truly recoverable with more graceful methods.
 *
 * @param nmemb the number of members to allocate
 * @param memsize the size of a single member of the type wanted to be allocated
 * @returns A guaranteed non-NULL pointer to the allocated memory block of the
 * size memsize * nmemb. All of the bytes in the function will be initialized to 0.
*/
void* tcalc_xcalloc(size_t, size_t);
/**
 * Extended realloc function which aborts the program on malloc failure
 *
 * Note that this may not be the best way to handle malloc failures in general
 * code, as interactive sessions could terminate unexpectedly which may have
 * been truly recoverable with more graceful methods.
 *
 * @param ptr The pointer that should be reallocated. This pointer could be
 * allocated with the C standard malloc or tcalc_xmalloc
 * @param size the size in bytes that the new memory block should be
 * @returns A guaranteed non-NULL pointer to the allocated memory block
*/
void* tcalc_xrealloc(void*, size_t);

/**
 * arr: The pointer to the array to perform possible growth on
 * size: The size to which the array should fit into
 * capacity: The current capacity of the array
 * err: a tcalc_err variable that will be set upon any errors
 *
 * the capacity variable will be updated to the current array's capacity if
 * reallocated, or will stay the same under no reallocation
 *
 * the arr variable will be altered, if reallocated, to point to the reallocated
 * data
*/
#define TCALC_DARR_GROW(arr, size, capacity, err) do { \
    if ((size) > (capacity)) { \
      const size_t grow_func_res = TCALC_ALLOC_NR(capacity); \
      if (grow_func_res < (size)) { \
        err = TCALC_ERR_OVERFLOW; \
      } else { \
        const size_t new_capacity = grow_func_res < (size) ? (size) : grow_func_res; \
        void* realloced = realloc((arr), sizeof(*(arr)) * new_capacity); \
        if (realloced == NULL) { \
          err = TCALC_ERR_BAD_ALLOC; \
        } else { \
          (arr) = realloced; \
          (capacity) = new_capacity; \
        } \
      } \
    } else { \
      err = TCALC_ERR_OK; \
    } \
  } while (0)

/**
 * arr: The pointer to the array to push val onto
 * size: The current size of the array
 * capacity: The current capacity of the array
 * val: The value to add to the array
 * err: a tcalc_err variable that will be set upon any errors
*/
#define TCALC_DARR_PUSH(arr, size, capacity, val, err) do { \
    TCALC_DARR_GROW(arr, (size) + 1, capacity, err); \
    if ((err) == TCALC_ERR_OK) { \
      (arr)[(size)++] = (val); \
    } \
  } while (0)

/**
 * arr: The pointer to the array to perform possible growth on
 * size: The current size of the array
 * capacity: The current capacity of the array
 * val: The value to add to the array
 * index: The index to insert the value at
 * err: a tcalc_err variable that will be set upon any errors
*/
#define TCALC_DARR_INSERT(arr, size, capacity, val, index, err) do { \
    assert((index) < (size) && (index) >= 0); \
    TCALC_DARR_GROW(arr, (size) + 1, capacity, err); \
    if ((err) == TCALC_ERR_OK) { \
      (size)++; \
      for (size_t i = (size) - 1; i > (index); i--) { \
        (arr)[i] = (arr)[i - 1]; \
      } \
      (arr)[index] = (val); \
    } \
  } while (0)

/**
 * Free the contents of an array with dynamically allocated members with a callback
 *
 * Free the contents of the array with a specific freeing function. The freeing
 * function should take one pointer which is the same type as that stored in the
 * array,
*/
#define TCALC_ARR_FREE_CF(arr, len, freefn) do { \
  for (size_t i = 0; i < (len); i++) { \
    freefn((arr)[i]); \
  } \
} while (0)

/**
 * Free the contents of an array with dynamically allocated members with a callback
 *
 * This is different from TCALC_ARR_FREE_CF, since the members will be casted to void*
 * first when feeding into the free function
 *
 * the signautre of the freeing function takes one void pointer as an argument
 * and returns null (the same as the C standard free function)
*/
#define TCALC_ARR_FREE_CFV(arr, len, freefnv) do { \
  for (size_t i = 0; i < (len); i++) { \
    freefnv((void*)((arr)[i])); \
  } \
} while (0)

/**
 * Free a dynamically allocated array and all of its contents, freeing the contents with a callback.
*/
#define TCALC_ARR_FREE_F(arr, len, freefn) do { \
    TCALC_ARR_FREE_CF(arr, len, freefn); \
    free(arr); \
    (arr) = NULL; \
  } while (0)

/**
 * Free a dynamically allocated array and all of its contents, freeing the
 * contents with a callback where each member of the array will first be
 * casted to void*.
*/
#define TCALC_ARR_FREE_FV(arr, len, freefnv) do { \
    TCALC_ARR_FREE_CFV(arr, len, freefnv); \
    free(arr); \
    (arr) = NULL; \
  } while (0)

#define TCALC_VEC(type) struct { \
    type* arr; \
    size_t len; \
    size_t cap; \
  }

#define TCALC_VEC_INIT { .arr = NULL, .len = 0, .cap = 0 }
#define TCALC_VEC_FREE(vec) do { \
    free((vec).arr); \
    (vec).arr = NULL; \
    (vec).len = 0; \
    (vec).cap = 0; \
  } while (0)


#define TCALC_VEC_PUSH(vec, item, err) TCALC_DARR_PUSH(vec.arr, vec.len, vec.cap, item, err)
#define TCALC_VEC_INSERT(vec, item, index, err) TCALC_DARR_INSERT(vec.arr, vec.len, vec.cap, item, index, err)
#define TCALC_VEC_GROW(vec, res_size, err) TCALC_DARR_GROW(vec.arr, res_size, vec.cap, err)
#define TCALC_VEC_FOREACH(vec, iname) for (size_t iname = 0; iname < vec.len; iname++)

#define TCALC_VEC_FREE_F(vec, freefn) do { \
  TCALC_ARR_FREE_F(vec.arr, vec.len, freefn); \
  TCALC_VEC_FREE(vec); \
} while (0)

#define TCALC_VEC_FREE_FV(vec, freefnv) do { \
  TCALC_ARR_FREE_F(vec.arr, vec.len, freefnv); \
  TCALC_VEC_FREE(vec); \
} while (0)



enum tcalc_valtype {
  TCALC_VALTYPE_NUM,
  TCALC_VALTYPE_BOOL
};

union tcalc_valunion {
  double num;
  bool boolean;
};

const char* tcalc_valtype_str(enum tcalc_valtype type);

/**
 * A representation for any value returned or held by a value within tcalc.
 * A tcalc_val can only either represent a boolean value or a numerical value.
 */
typedef struct tcalc_val {
  enum tcalc_valtype type;
  union tcalc_valunion as;
} tcalc_val;

#define TCALC_VAL_INIT_NUM(_double_expr_) \
  (struct tcalc_val){ \
    .type = TCALC_VALTYPE_NUM, \
    .as = { \
      .num = (_double_expr_) \
    } \
  }

#define TCALC_VAL_INIT_BOOL(_boolean_expr_) \
  (struct tcalc_val){ \
    .type = TCALC_VALTYPE_BOOL, \
    .as = { \
      .boolean = (_boolean_expr_) \
    } \
  }

typedef tcalc_err (*tcalc_unfunc)(double, double*);
typedef tcalc_err (*tcalc_binfunc)(double, double, double*);
typedef bool (*tcalc_relfunc)(double, double);

// l suffix stands for "logical" in tcalc_unlfunc and tcalc_binlfunc

typedef bool (*tcalc_unlfunc)(bool);
typedef bool (*tcalc_binlfunc)(bool, bool);


// Logical Functions

bool tcalc_not(bool a);
bool tcalc_and(bool a, bool b);
bool tcalc_or(bool a, bool b);
bool tcalc_nand(bool a, bool b);
bool tcalc_nor(bool a, bool b);
bool tcalc_xor(bool a, bool b);
bool tcalc_xnor(bool a, bool b);
bool tcalc_matcond(bool a, bool b);
bool tcalc_equals_l(bool a, bool b);
bool tcalc_nequals_l(bool a, bool b);

// Relational Functions

bool tcalc_equals(double a, double b);
bool tcalc_nequals(double a, double b);
bool tcalc_lt(double a, double b);
bool tcalc_lteq(double a, double b);
bool tcalc_gt(double a, double b);
bool tcalc_gteq(double a, double b);


// Unary functions

tcalc_err tcalc_ceil(double a, double* out);
tcalc_err tcalc_floor(double a, double* out);
tcalc_err tcalc_round(double a, double* out);
tcalc_err tcalc_abs(double a, double* out);



// Radian Trigonometric Functions

tcalc_err tcalc_sin(double a, double* out);
tcalc_err tcalc_cos(double a, double* out);
tcalc_err tcalc_tan(double a, double* out);
tcalc_err tcalc_sec(double a, double* out);
tcalc_err tcalc_csc(double a, double* out);
tcalc_err tcalc_cot(double a, double* out);

tcalc_err tcalc_asin(double a, double* out);
tcalc_err tcalc_acos(double a, double* out);
tcalc_err tcalc_atan(double a, double* out);
tcalc_err tcalc_atan2(double a, double b, double* out);
tcalc_err tcalc_asec(double a, double* out);
tcalc_err tcalc_acsc(double a, double* out);
tcalc_err tcalc_acot(double a, double* out);

tcalc_err tcalc_sinh(double a, double* out);
tcalc_err tcalc_cosh(double a, double* out);
tcalc_err tcalc_tanh(double a, double* out);
tcalc_err tcalc_asinh(double a, double* out);
tcalc_err tcalc_acosh(double a, double* out);
tcalc_err tcalc_atanh(double a, double* out);

// Degree trigonometric functions

tcalc_err tcalc_sin_deg(double a, double* out);
tcalc_err tcalc_cos_deg(double a, double* out);
tcalc_err tcalc_tan_deg(double a, double* out);
tcalc_err tcalc_sec_deg(double a, double* out);
tcalc_err tcalc_csc_deg(double a, double* out);
tcalc_err tcalc_cot_deg(double a, double* out);

tcalc_err tcalc_asin_deg(double a, double* out);
tcalc_err tcalc_acos_deg(double a, double* out);
tcalc_err tcalc_atan_deg(double a, double* out);
tcalc_err tcalc_atan2_deg(double a, double b, double* out);
tcalc_err tcalc_asec_deg(double a, double* out);
tcalc_err tcalc_acsc_deg(double a, double* out);
tcalc_err tcalc_acot_deg(double a, double* out);

tcalc_err tcalc_sinh_deg(double a, double* out);
tcalc_err tcalc_cosh_deg(double a, double* out);
tcalc_err tcalc_tanh_deg(double a, double* out);
tcalc_err tcalc_asinh_deg(double a, double* out);
tcalc_err tcalc_acosh_deg(double a, double* out);
tcalc_err tcalc_atanh_deg(double a, double* out);


tcalc_err tcalc_unary_plus(double a, double* out);
tcalc_err tcalc_unary_minus(double a, double* out);
tcalc_err tcalc_log(double a, double* out);
tcalc_err tcalc_sqrt(double a, double* out);
tcalc_err tcalc_cbrt(double a, double* out);

tcalc_err tcalc_ln(double a, double* out);
tcalc_err tcalc_exp(double a, double* out);

// Binary functions:

tcalc_err tcalc_add(double a, double b, double* out);
tcalc_err tcalc_subtract(double a, double b, double* out);
tcalc_err tcalc_multiply(double a, double b, double* out);
tcalc_err tcalc_divide(double a, double b, double* out);
tcalc_err tcalc_mod(double a, double b, double* out);

tcalc_err tcalc_pow(double a, double b, double* out);


typedef tcalc_err (*tcalc_val_unfunc)(tcalc_val, double*);
typedef tcalc_err (*tcalc_val_binfunc)(tcalc_val, tcalc_val, double*);
typedef tcalc_err (*tcalc_val_relfunc)(tcalc_val, tcalc_val, bool*);

// l suffix stands for "logical"
typedef tcalc_err (*tcalc_val_unlfunc)(tcalc_val, bool* out);
typedef tcalc_err (*tcalc_val_binlfunc)(tcalc_val, tcalc_val, bool* out);


/**
 * Logical Functions
*/
tcalc_err tcalc_val_not(tcalc_val a, bool* out);
tcalc_err tcalc_val_and(tcalc_val a, tcalc_val b, bool* out);
tcalc_err tcalc_val_or(tcalc_val a, tcalc_val b, bool* out);
tcalc_err tcalc_val_nand(tcalc_val a, tcalc_val b, bool* out);
tcalc_err tcalc_val_nor(tcalc_val a, tcalc_val b, bool* out);
tcalc_err tcalc_val_xor(tcalc_val a, tcalc_val b, bool* out);
tcalc_err tcalc_val_xnor(tcalc_val a, tcalc_val b, bool* out);
tcalc_err tcalc_val_matcond(tcalc_val a, tcalc_val b, bool* out);
tcalc_err tcalc_val_equals_l(tcalc_val a, tcalc_val b, bool* out);
tcalc_err tcalc_val_nequals_l(tcalc_val a, tcalc_val b, bool* out);

/**
 * Relational Functions
*/
tcalc_err tcalc_val_equals(tcalc_val a, tcalc_val b, bool* out);
tcalc_err tcalc_val_nequals(tcalc_val a, tcalc_val b, bool* out);
tcalc_err tcalc_val_lt(tcalc_val a, tcalc_val b, bool* out);
tcalc_err tcalc_val_lteq(tcalc_val a, tcalc_val b, bool* out);
tcalc_err tcalc_val_gt(tcalc_val a, tcalc_val b, bool* out);
tcalc_err tcalc_val_gteq(tcalc_val a, tcalc_val b, bool* out);


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

typedef struct tcalc_slice {
  size_t start;
  size_t xend;
} tcalc_slice;


static inline size_t tcalc_slice_len(tcalc_slice slice) {
  return slice.xend - slice.start;
}

#define TCALC_STRLIT_LEN(strlit) (sizeof(strlit) - 1)
#define TCALC_STRLIT_PTR_LEN(strlit) ((strlit)), TCALC_STRLIT_LEN(strlit)

bool tcalc_streq_lblb(const char* s1, size_t l1, const char* s2, size_t l2);

// Check if a null terminated string and a length-based string hold equivalent
// information
bool tcalc_streq_ntlb(const char* ntstr, const char* lbstr, size_t lbstr_len);

bool tcalc_slice_ntstr_eq(const char* source, tcalc_slice slice, const char* ntstr);

size_t tcalc_strlcpy(char *dst, const char *src, size_t dsize);

bool tcalc_lpstrisdouble(const char*, size_t);
enum tcalc_err tcalc_lpstrtodouble(const char*, size_t, double*);

bool tcalc_str_list_has(const char* input, const char** list, size_t count);

bool tcalc_strhaspre(const char* prefix, const char* str);

/*
Allowed TCalc Tokens:
All Alphanumeric Characters
"+-/%*^" operators
"()" Parentheses
"." decimal point
" " whitespace (will be ignored)

Tokens will be separated based on:
- strings of unsigned integers
- strings of unsigned decimal numbers
- strings of lowercase letters
- operators
- grouping symbols

Whitespace will not be included in the actual token output, but can affect what
tokens are interpreted as separate. For example:

Strings of characters will be disambiguated by the actual parser for the tokens.
The way that the characters are disambiguated will be up to the parser, as a string
of characters could simultaneously represent

Since names of functions and variables will definitely be disambiguous if multiplication
is allowed to be represented through placing variables next to each other (xy),
all strings of alphanumeric characters and "." characters will remain untouched.

I thought it best to give equality it's own operator type to simplify parsing.
Note that this also applies to !=

Valid Examples:

"(54+23) * 34"
["(", "54", "+", "23", ")", "*", "34"]

"2^(3+3.5)/3"
*/

typedef enum tcalc_token_type {
  TCALC_TOK_NUM, // A parsable number
  TCALC_TOK_UNOP, // unary operator ("+", "-")
  TCALC_TOK_BINOP, // binary operator ("+", "-", "*", "/", "^", "**")
  TCALC_TOK_RELOP, // relational (binary) operator ("<", "<=", ">", ">=")
  TCALC_TOK_UNLOP, // unary logical operator ("!")
  TCALC_TOK_BINLOP, // binary logical operator ("&&", "||")
  TCALC_TOK_EQOP, // equality operator ("==", "=", "!=")
  TCALC_TOK_PSEP, // parameter separator (",")
  TCALC_TOK_ID, // identifier ([a-zA-Z_][a-zA-Z0-9_]*)
  TCALC_TOK_GRPSTRT, // starting token for a pair of grouping symbols ("(")
  TCALC_TOK_GRPEND, // ending token for a pair of grouping symbols (")")
  TCALC_TOK_EOF // end of file
} tcalc_token_type;

const char* tcalc_token_type_str(tcalc_token_type token_type);

// typedef struct tcalc_token {
//   tcalc_token_type type;
//   char* val;
// } tcalc_token;


// Data that a token can contain:
// Type
// Starting Offset
// Ending Offset
// Instead of a pointer to the string, provide a starting and ending index.
// This would allow for the area around strings in the data to be easily
// displayed if necessary.
// Line data can just be calculated later by iterating over the source string
// if it is needed. Line data should not be needed unless an error occurs, so
// there's no good reason to store it.

typedef struct tcalc_token {
  tcalc_token_type type;
  size_t start;
  size_t xend;
} tcalc_token;

inline static const char* tcalc_token_startcp(const char* str, tcalc_token tok) { return str + tok.start; }
inline static char* tcalc_token_startp(char* str, tcalc_token tok) { return str + tok.start; }
inline static size_t tcalc_token_len(tcalc_token tok) { return tok.xend - tok.start; }

tcalc_err tcalc_tokenize_infix(const char* expr, tcalc_token** out, size_t* out_size);

#define TCALC_TOKEN_IMPLICIT_MULT_PRINTF_STR ("*")

#define TCALC_TOKEN_IS_IMPLICIT_MULT(token) ((token).type == TCALC_TOK_BINOP && tcalc_token_len((token)) == 0)

// Must use %.*s with tokens. This macro simplifies adding the token length
// and string information into the variable arguments section of printf formats.
// Also, it makes sure that the length is casted to a integer, like it
// should be, which is easy to forget
//
// Handles printing implicit multiplication
#define TCALC_TOKEN_PRINTF_VARARG(expr, token) \
  (TCALC_TOKEN_IS_IMPLICIT_MULT(token) ? (int)TCALC_STRLIT_LEN(TCALC_TOKEN_IMPLICIT_MULT_PRINTF_STR) : (int)tcalc_token_len((token))), \
  (TCALC_TOKEN_IS_IMPLICIT_MULT(token) ? (TCALC_TOKEN_IMPLICIT_MULT_PRINTF_STR) : tcalc_token_startcp((expr), (token)))

// Must use %.*s with tokens. This macro simplifies adding the token length
// and string information into the variable arguments section of printf formats.
// Also, it makes sure that the length is casted to a integer, like it
// should be, which is easy to forget
//
// Implicit multiplication is not handled as a special case.
#define TCALC_TOKEN_PRINTF_VARARG_EXACT(expr, token) \
  (int)tcalc_token_len((token)), tcalc_token_startcp((expr), (token))

struct tcalc_ctx;
struct tcalc_val;

typedef struct tcalc_exprtree_node tcalc_exprtree_node;
typedef struct tcalc_exprtree_node tcalc_exprtree;

// Valid Token Types:
// TCALC_TOK_ID: identifier for a binary function
// TCALC_TOK_BINOP: operator for a binary operator
// TCALC_TOK_BINLOP: operator for a binary logical operator
// TCALC_TOK_RELOP: operator for a binary relational operator
// TCALC_TOK_EQOP: operator for a binary equality operator
typedef struct tcalc_exprtree_binary_node {
  struct tcalc_token token;
  tcalc_exprtree_node* left;
  tcalc_exprtree_node* right;
} tcalc_exprtree_binary_node;

// Valid Token Types:
// TCALC_TOK_ID: identifier for a unary function
// TCALC_TOK_UNOP: operator for a unary operator
// TCALC_TOK_UNLOP: operator for a unary logical operator
typedef struct tcalc_exprtree_unary_node {
  struct tcalc_token token;
  tcalc_exprtree_node* child;
} tcalc_exprtree_unary_node;


// Valid Token Types:
// TCALC_TOK_ID: identifier for a variable
// TCALC_TOK_NUM: Numerical string
typedef struct tcalc_exprtree_value_node {
  struct tcalc_token token;
} tcalc_exprtree_value_node;


#if 0
// Currently Unused
// Exists in source code to show that the size of tcalc_exprtree_func_node when
// arbitrary sized functions are supported would be equal to the size of
// tcalc_exprtree_binary_node, making the union of the two waste no memory
typedef struct tcalc_exprtree_func_node {
  struct tcalc_token* token;
  struct tcalc_exprtree_node** children;
  size_t nb_children;
} tcalc_exprtree_func_node;
#endif

enum tcalc_exprtree_node_type {
  TCALC_EXPRTREE_NODE_TYPE_BINARY,
  TCALC_EXPRTREE_NODE_TYPE_UNARY,
  TCALC_EXPRTREE_NODE_TYPE_VALUE,
  // TCALC_EXPRTREE_NODE_TYPE_FUNC
};

struct tcalc_exprtree_node {
  enum tcalc_exprtree_node_type type;
  union {
    tcalc_exprtree_binary_node binary;
    tcalc_exprtree_unary_node unary;
    // tcalc_exprtree_func_node func;
    tcalc_exprtree_value_node value;
  } as;
};

/**
 * Free a tcalc expression tree **recursively**
 * This function can be called even if 'head' is NULL, any of the
 * children of 'head' are NULL, or the token of 'head' is NULL
*/
void tcalc_exprtree_free(tcalc_exprtree* head);

/**
 * Free a tcalc expression tree's children **recursively** and set each
 * child to NULL.
 *
 * defined as no-op if head == NULL
*/
void tcalc_exprtree_free_children(tcalc_exprtree* head);

tcalc_err tcalc_create_exprtree_infix(const char* expr, const struct tcalc_ctx* ctx, tcalc_exprtree** out);

tcalc_err tcalc_eval_exprtree(const char* expr, tcalc_exprtree* exprtree, const struct tcalc_ctx* ctx, struct tcalc_val* out);

/**
 * Notes on precedence:
 *
 * A higher precedence value means that that operation will take place before
 * operations of lower precedence values. For example, if multiplication has
 * a precedence value of 3, exponentiation has a precedence value of 4, and
 * addition has a precedence value of 2, exponentiation always comes first, then
 * multiplication, then addition when evaluating an expression within the same
 * grouping symbols.
 *
 * Higher Precedence Value <- Higher Precedence <- Lower down AST
 *
 */

/**
 * Precedence Rankings (Non-Numerical):
 *
 * Highest Precedence, Handled First
 * ---------------------
 * "^", "**" (Exponentiation)
 * ---------------------
 * "+" (Unary Plus)
 * "-" (Unary Minus)
 * "!" (Logical NOT)
 * ---------------------
 * "*" (Multiplication)
 * "/" (Division)
 * "%" (Modulus)
 * ---------------------
 * "+" (Addition)
 * "-" (Subtraction)
 * ---------------------
 * "<" (Less Than)
 * "<=" (Less Than or Equal To)
 * ">" (Greater Than)
 * ">=" (Greater Than or Equal To)
 * ---------------------
 * "=" (Relational Equals)
 * "==" (Relational Equals)
 * "!=" (Relational Not Equals)
 * "=" (Logical Equals)
 * "==" (Logical Equals)
 * "!=" (Logical Not Equals)
 * ---------------------
 * "&&" (Logical AND)
 * ---------------------
 * "||" (Logical OR)
 * ---------------------
 * Lowest Precedence, Handled Last
*/

typedef enum tcalc_assoc{
  TCALC_RIGHT_ASSOC,
  TCALC_LEFT_ASSOC,
} tcalc_assoc;

typedef struct tcalc_opdata {
  int prec;
  tcalc_assoc assoc;
} tcalc_opdata;


// 8 may seem large, but because of alignment from function pointers,
// each operator id struct must be aligned across the alignment of a function
// pointer, which is 8 bytes on 64 bit machines. If we used less than 8, it
// would be wasting bytes as padding instead
#define TCALC_OPDEF_MAX_STR_SIZE 8

#define TCALC_IDDEF_MAX_STR_SIZE 64

/**
 * Maybe the tcalc_ctx should not hold information about user-defined functions,
 * but only the actual definitions. Something else could handle resolving
 * user-defined variable definitions?
 *
 * We have to hold our expression as
*/
typedef struct tcalc_exprvardef {
  char id[TCALC_IDDEF_MAX_STR_SIZE];
  const char* expr;
} tcalc_exprvardef;

typedef struct tcalc_unopdef {
  char id[TCALC_OPDEF_MAX_STR_SIZE];
  int prec;
  tcalc_assoc assoc;
  tcalc_val_unfunc func;
} tcalc_unopdef;

typedef struct tcalc_binopdef {
  char id[TCALC_OPDEF_MAX_STR_SIZE];
  int prec;
  tcalc_assoc assoc;
  tcalc_val_binfunc func;
} tcalc_binopdef;

typedef struct tcalc_relopdef {
  char id[TCALC_OPDEF_MAX_STR_SIZE];
  int prec;
  tcalc_assoc assoc;
  tcalc_val_relfunc func;
} tcalc_relopdef;

typedef struct tcalc_unlopdef {
  char id[TCALC_OPDEF_MAX_STR_SIZE];
  int prec;
  tcalc_assoc assoc;
  tcalc_val_unlfunc func;
} tcalc_unlopdef;

typedef struct tcalc_binlopdef {
  char id[TCALC_OPDEF_MAX_STR_SIZE];
  int prec;
  tcalc_assoc assoc;
  tcalc_val_binlfunc func;
} tcalc_binlopdef;

typedef struct tcalc_vardef {
  char id[TCALC_IDDEF_MAX_STR_SIZE];
  struct tcalc_val val;
} tcalc_vardef;


typedef struct tcalc_unfuncdef {
  char id[TCALC_IDDEF_MAX_STR_SIZE];
  tcalc_val_unfunc func;
} tcalc_unfuncdef;

typedef struct tcalc_binfuncdef {
  char id[TCALC_IDDEF_MAX_STR_SIZE];
  tcalc_val_binfunc func;
} tcalc_binfuncdef;

/**
 * Some conditions to a tcalc context:
 *
 * Unary function identifiers must only contain alphabetical characters
 * Binary function identifiers must only contain alphabetical characters
 * Variable identifiers must only contain aphabetical characters
 *
 * Symbols and identifiers must NOT be the same across the tcalc_ctx
 * - The only exception to the above rule is that unary operators and binary
 *   operators can have the same symbol identifiers. This should be taken with
 *   salt though, as the tokenizer will disambiguate itself which + or -
 *   are unary and which + or - are binary. If I had created math I wouldn't
 *   have created it like this. -_-
*/
typedef struct tcalc_ctx {
  TCALC_VEC(tcalc_unfuncdef) unfuncs; // Defined Unary Functions
  TCALC_VEC(tcalc_binfuncdef) binfuncs; // Defined Binary Functions
  TCALC_VEC(tcalc_vardef) vars; // Defined Variables
  TCALC_VEC(tcalc_unopdef) unops; // Defined Unary Operators
  TCALC_VEC(tcalc_binopdef) binops; // Defined Binary Operators
  TCALC_VEC(tcalc_relopdef) relops; // Defined Relational (Binary) Operators
  TCALC_VEC(tcalc_unlopdef) unlops; // Defined Logical Unary Operators
  TCALC_VEC(tcalc_binlopdef) binlops; // Defined Logical Binary Operators
} tcalc_ctx;

tcalc_err tcalc_ctx_alloc_empty(tcalc_ctx** out);
tcalc_err tcalc_ctx_alloc_default(tcalc_ctx** out);

tcalc_err tcalc_ctx_addtrigrad(tcalc_ctx* ctx);
tcalc_err tcalc_ctx_addtrigdeg(tcalc_ctx* ctx);

void tcalc_ctx_free(tcalc_ctx* ctx);

tcalc_err tcalc_ctx_addvar(tcalc_ctx* ctx, const char* name, size_t name_len, struct tcalc_val val);
tcalc_err tcalc_ctx_addunfunc(tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_val_unfunc func);
tcalc_err tcalc_ctx_addbinfunc(tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_val_binfunc func);
tcalc_err tcalc_ctx_addunop(tcalc_ctx* ctx, const char* name, size_t name_len, int prec, tcalc_assoc assoc, tcalc_val_unfunc func);
tcalc_err tcalc_ctx_addbinop(tcalc_ctx* ctx, const char* name, size_t name_len, int prec, tcalc_assoc assoc, tcalc_val_binfunc func);
tcalc_err tcalc_ctx_addrelop(tcalc_ctx* ctx,const char* name, size_t name_len, int prec, tcalc_assoc assoc, tcalc_val_relfunc func);
tcalc_err tcalc_ctx_addunlop(tcalc_ctx* ctx,const char* name, size_t name_len, int prec, tcalc_assoc assoc, tcalc_val_unlfunc func);
tcalc_err tcalc_ctx_addbinlop(tcalc_ctx* ctx, const char* name, size_t name_len, int prec, tcalc_assoc assoc, tcalc_val_binlfunc func);

/**
 * Note that tcalc_ctx_hasid does not apply to operations, but only
 * functions and variables
*/
bool tcalc_ctx_hasop(const tcalc_ctx* ctx, const char* name, size_t name_len);
bool tcalc_ctx_hasid(const tcalc_ctx* ctx, const char* name, size_t name_len);
bool tcalc_ctx_hasfunc(const tcalc_ctx* ctx, const char* name, size_t name_len);

bool tcalc_ctx_hasunop(const tcalc_ctx* ctx, const char* name, size_t name_len);
bool tcalc_ctx_hasbinop(const tcalc_ctx* ctx, const char* name, size_t name_len);
bool tcalc_ctx_hasrelop(const tcalc_ctx* ctx, const char* name, size_t name_len);
bool tcalc_ctx_hasunlop(const tcalc_ctx* ctx, const char* name, size_t name_len);
bool tcalc_ctx_hasbinlop(const tcalc_ctx* ctx, const char* name, size_t name_len);

bool tcalc_ctx_hasbinfunc(const tcalc_ctx* ctx, const char* name, size_t name_len);
bool tcalc_ctx_hasunfunc(const tcalc_ctx* ctx, const char* name, size_t name_len);

bool tcalc_ctx_hasvar(const tcalc_ctx* ctx, const char* name, size_t name_len);

/**
 * Note that out must be NON-NULL
 *
 * tcalc_ctx_get_x functions will not return an error whenever the same given
 * context and name parameters return truthy from their corresponding
 * tcalc_ctx_has_x functions (if tcalc_ctx_has_x returns true, tcalc_ctx_get_x
 * will not return an error)
*/
tcalc_err tcalc_ctx_getvar(const tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_vardef* out);


tcalc_err tcalc_ctx_getunfunc(const tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_unfuncdef* out);
tcalc_err tcalc_ctx_getbinfunc(const tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_binfuncdef* out);

tcalc_err tcalc_ctx_getunop(const tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_unopdef* out);
tcalc_err tcalc_ctx_getbinop(const tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_binopdef* out);
tcalc_err tcalc_ctx_getrelop(const tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_relopdef* out);
tcalc_err tcalc_ctx_getunlop(const tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_unlopdef* out);
tcalc_err tcalc_ctx_getbinlop(const tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_binlopdef* out);

/**
 * Note that since a variable symbol can be defined as multiple different operator
 * types, such as unary and binary "+", having a general function to fetch
 * operator precedence and associativity does not serve as useful a purpose
 * as it first seems
*/
// tcalc_err tcalc_ctx_getopdata(const tcalc_ctx* ctx, const char* name, tcalc_opdata* out);


tcalc_err tcalc_eval_wctx(const char* infix, const struct tcalc_ctx* ctx, struct tcalc_val* out);
tcalc_err tcalc_eval(const char* infix, struct tcalc_val* out);

#endif
