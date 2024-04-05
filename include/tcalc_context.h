#ifndef TCALC_CONTEXT_H
#define TCALC_CONTEXT_H

#include "tcalc_error.h"
#include "tcalc_func.h"
#include "tcalc_vec.h"

#include <stddef.h>

struct tcalc_token;
struct tcalc_exprtree_node;

typedef enum tcalc_assoc{
  TCALC_RIGHT_ASSOC,
  TCALC_LEFT_ASSOC,
} tcalc_assoc;

typedef struct tcalc_vardef {
  char* id;
  double val;
} tcalc_vardef;

typedef struct tcalc_opdata {
  int prec;
  tcalc_assoc assoc;
} tcalc_opdata;

typedef struct tcalc_exprvardef {
  char* name;
  struct tcalc_exprtree_node* expr;
  char* dependencies;
} tcalc_exprvardef;


typedef struct tcalc_uopdef {
  char* id;
  int prec;
  tcalc_assoc assoc;
  tcalc_unfunc func;
} tcalc_uopdef;

typedef struct tcalc_relopdef {
  char* id;
  int prec;
  tcalc_assoc assoc;
  tcalc_relfunc func;
} tcalc_relopdef;

typedef struct tcalc_logiopdef {
  char* id;
  int prec;
  tcalc_assoc assoc;
  tcalc_logifunc func;
} tcalc_logopdef;

typedef struct tcalc_binopdef {
  char* id;
  int prec;
  tcalc_assoc assoc;
  tcalc_binfunc func;
} tcalc_binopdef;

typedef struct tcalc_unfuncdef {
  char* id;
  tcalc_unfunc func;
} tcalc_unfuncdef;

typedef struct tcalc_binfuncdef {
  char* id;
  tcalc_binfunc func;
} tcalc_binfuncdef;

typedef struct tcalc_groupsymdef {
  char start_symbol;
  char end_symbol;
} tcalc_groupsymdef;

tcalc_opdata tcalc_getbinopdata(tcalc_binopdef* binary_op_def);
tcalc_opdata tcalc_getunopdata(tcalc_uopdef* unary_op_def);

/**
 * Some conditions to a tcalc context:
 * 
 * Unary function identifiers must only contain alphabetical characters
 * Binary function identifiers must only contain alphabetical characters
 * Variable identifiers must only contain aphabetical characters
 * 
 * Symbols and identifiers must NOT be the same across the tcalc_ctx 
 * - The only exception to the above rule is that unary operators and binary operators
 *  can have the same symbol identifiers. This should be taken with salt though, as the tokenizer
 *  will disambiguate itself which + or - are unary and which + or - are binary. 
 *  If I had created math I wouldn't have created it like this.
*/
typedef struct tcalc_ctx {
  TCALC_VEC(tcalc_unfuncdef*) unfuncs;
  TCALC_VEC(tcalc_binfuncdef*) binfuncs;
  TCALC_VEC(tcalc_vardef*) vars;
  TCALC_VEC(tcalc_uopdef*) unops;
  TCALC_VEC(tcalc_binopdef*) binops;
  TCALC_VEC(tcalc_relopdef*) relops;
} tcalc_ctx;

tcalc_err tcalc_ctx_alloc_empty(tcalc_ctx** out);
tcalc_err tcalc_ctx_alloc_default(tcalc_ctx** out);

void tcalc_ctx_free(tcalc_ctx* ctx);

tcalc_err tcalc_ctx_addvar(tcalc_ctx* ctx, char* name, double val);
tcalc_err tcalc_ctx_addunfunc(tcalc_ctx* ctx, char* name, tcalc_unfunc func);
tcalc_err tcalc_ctx_addbinfunc(tcalc_ctx* ctx, char* name, tcalc_binfunc func);
tcalc_err tcalc_ctx_addunop(tcalc_ctx* ctx, char* name, int prec, tcalc_assoc assoc, tcalc_unfunc func);
tcalc_err tcalc_ctx_addbinop(tcalc_ctx* ctx,  char* name, int prec, tcalc_assoc assoc, tcalc_binfunc func);

int tcalc_ctx_hasid(const tcalc_ctx* ctx, const char* name);
int tcalc_ctx_hasfunc(const tcalc_ctx* ctx, const char* name);

int tcalc_ctx_hasop(const tcalc_ctx* ctx, const char* name);
int tcalc_ctx_hasunop(const tcalc_ctx* ctx, const char* name);
int tcalc_ctx_hasbinop(const tcalc_ctx* ctx, const char* name);
int tcalc_ctx_hasunfunc(const tcalc_ctx* ctx, const char* name);
int tcalc_ctx_hasbinfunc(const tcalc_ctx* ctx, const char* name);
int tcalc_ctx_hasvar(const tcalc_ctx* ctx, const char* name);

/**
 * tcalc_ctx_get_x functions will not return an error whenever the same given
 * context and name parameters return truthy from their corresponding
 * tcalc_ctx_has_x functions (if tcalc_ctx_has_x returns true, tcalc_ctx_get_x
 * will not return an error)
*/

tcalc_err tcalc_ctx_getunfunc(const tcalc_ctx* ctx, const char* name, tcalc_unfuncdef** out);
tcalc_err tcalc_ctx_getbinfunc(const tcalc_ctx* ctx, const char* name, tcalc_binfuncdef** out);
tcalc_err tcalc_ctx_getvar(const tcalc_ctx* ctx, const char* name, tcalc_vardef** out);
tcalc_err tcalc_ctx_getunop(const tcalc_ctx* ctx, const char* name, tcalc_uopdef** out);
tcalc_err tcalc_ctx_getbinop(const tcalc_ctx* ctx, const char* name, tcalc_binopdef** out);

tcalc_err tcalc_ctx_get_op_data(const tcalc_ctx* ctx, const char* name, tcalc_opdata* out);

#endif