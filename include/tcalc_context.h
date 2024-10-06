#ifndef TCALC_CONTEXT_H
#define TCALC_CONTEXT_H

#include "tcalc_error.h"
#include "tcalc_func.h"
#include "tcalc_vec.h"
#include "tcalc_val_func.h"

#include <stdbool.h>
#include <stddef.h>

struct tcalc_token;
struct tcalc_exprtree_node;

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


#endif
