#include "tcalc_context.h"

#include "tcalc_error.h"
#include "tcalc_constants.h"
#include "tcalc_func.h"
#include "tcalc_mem.h"
#include "tcalc_string.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

tcalc_err tcalc_vardef_alloc(const char* name, double val, tcalc_vardef** out);
tcalc_err tcalc_lvardef_alloc(const char* name, int boolval, tcalc_lvardef** out);
tcalc_err tcalc_binfuncdef_alloc(const char* name, tcalc_binfunc func, tcalc_binfuncdef** out);
tcalc_err tcalc_unfuncdef_alloc(const char* name, tcalc_unfunc func, tcalc_unfuncdef** out);
tcalc_err tcalc_binopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_binfunc func, tcalc_binopdef** out);
tcalc_err tcalc_unopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_unfunc func, tcalc_unopdef** out);
tcalc_err tcalc_relopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_relfunc func, tcalc_relopdef** out);
tcalc_err tcalc_unlopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_unlfunc func, tcalc_unlopdef** out);
tcalc_err tcalc_binlopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_binlfunc func, tcalc_binlopdef** out);

void tcalc_vardef_free(tcalc_vardef* var_def);
void tcalc_lvardef_free(tcalc_lvardef* var_def);
void tcalc_binfuncdef_free(tcalc_binfuncdef* binary_func_def);
void tcalc_unfuncdef_free(tcalc_unfuncdef* unary_func_def);
void tcalc_binopdef_free(tcalc_binopdef* binary_op_def);
void tcalc_unopdef_free(tcalc_unopdef* unary_op_def);
void tcalc_relopdef_free(tcalc_relopdef* rel_op_def);
void tcalc_unlopdef_free(tcalc_unlopdef* unl_op_def);
void tcalc_binlopdef_free(tcalc_binlopdef* binl_op_def);

tcalc_err tcalc_ctx_alloc_empty(tcalc_ctx** out) {
  // use of calloc is important here! We have to null all of the TCALC_VEC 
  // structs inside the ctx.
  tcalc_ctx* ctx = (tcalc_ctx*)calloc(1, sizeof(tcalc_ctx));
  if (ctx == NULL) return TCALC_BAD_ALLOC;

  *out = ctx;
  return TCALC_OK; 
}

tcalc_err tcalc_ctx_alloc_default(tcalc_ctx** out) {
  tcalc_ctx* ctx;
  tcalc_err err = tcalc_ctx_alloc_empty(&ctx);
  if (err) return err;

  // Default Unary Functions:
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "sin", tcalc_sin));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "cos", tcalc_cos));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "tan", tcalc_tan));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "sec", tcalc_sec));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "csc", tcalc_csc));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "cot", tcalc_cot));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "asin", tcalc_asin));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "arcsin", tcalc_asin));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "acos", tcalc_acos));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "arccos", tcalc_acos));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "atan", tcalc_atan));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "arctan", tcalc_atan));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "asec", tcalc_asec));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "arcsec", tcalc_asec));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "acsc", tcalc_acsc));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "arccsc", tcalc_acsc));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "acot", tcalc_acot));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "arccot", tcalc_acot));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "sinh", tcalc_sinh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "cosh", tcalc_cosh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "tanh", tcalc_tanh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "asinh", tcalc_asinh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "arcsinh", tcalc_asinh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "acosh", tcalc_acosh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "arccosh", tcalc_acosh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "atanh", tcalc_atanh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "arctanh", tcalc_atanh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "log", tcalc_log));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "ln", tcalc_ln));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "exp", tcalc_exp));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "sqrt", tcalc_sqrt));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "cbrt", tcalc_cbrt));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "ceil", tcalc_ceil));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "floor", tcalc_floor));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "round", tcalc_round));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "abs", tcalc_abs));

  // Default Binary Functions:
  cleanup_on_err(err, tcalc_ctx_addbinfunc(ctx, "pow", tcalc_pow));

  // Default Variables:
  cleanup_on_err(err, tcalc_ctx_addvar(ctx, "pi", M_PI));
  cleanup_on_err(err, tcalc_ctx_addvar(ctx, "e", M_E));

  // Default Logical Variables:
  cleanup_on_err(err, tcalc_ctx_addlvar(ctx, "true", 1));
  cleanup_on_err(err, tcalc_ctx_addlvar(ctx, "false", 0));

  // Default Logical Unary Operators:
  cleanup_on_err(err, tcalc_ctx_addunlop(ctx, "!", 10, TCALC_RIGHT_ASSOC, tcalc_not));

  // Default Unary Operators:
  cleanup_on_err(err, tcalc_ctx_addunop(ctx, "+", 10, TCALC_RIGHT_ASSOC, tcalc_unary_plus));
  cleanup_on_err(err, tcalc_ctx_addunop(ctx, "-", 10, TCALC_RIGHT_ASSOC, tcalc_unary_minus));

  // Default Binary Operators:
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "**", 10, TCALC_RIGHT_ASSOC, tcalc_pow));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "^", 10, TCALC_RIGHT_ASSOC, tcalc_pow));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "*", 9, TCALC_LEFT_ASSOC, tcalc_multiply));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "/", 9, TCALC_LEFT_ASSOC, tcalc_divide));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "%", 9, TCALC_LEFT_ASSOC, tcalc_mod));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "+", 8, TCALC_LEFT_ASSOC, tcalc_add));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "-", 8, TCALC_LEFT_ASSOC, tcalc_subtract));

  // Default Relational Binary Operators:
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, "<", 6, TCALC_LEFT_ASSOC, tcalc_lt));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, "<=", 6, TCALC_LEFT_ASSOC, tcalc_lteq));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, ">", 6, TCALC_LEFT_ASSOC, tcalc_gt));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, ">=", 6, TCALC_LEFT_ASSOC, tcalc_gteq));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, "=", 5, TCALC_RIGHT_ASSOC, tcalc_equals));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, "==", 5, TCALC_RIGHT_ASSOC, tcalc_equals));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, "!=", 5, TCALC_RIGHT_ASSOC, tcalc_nequals));
  

  
  // Default Logical Binary Operators:
  cleanup_on_err(err, tcalc_ctx_addbinlop(ctx, "=", 5, TCALC_RIGHT_ASSOC, tcalc_equals_l));
  cleanup_on_err(err, tcalc_ctx_addbinlop(ctx, "==", 5, TCALC_RIGHT_ASSOC, tcalc_equals_l));
  cleanup_on_err(err, tcalc_ctx_addbinlop(ctx, "!=", 5, TCALC_RIGHT_ASSOC, tcalc_nequals_l));
  cleanup_on_err(err, tcalc_ctx_addbinlop(ctx, "&&", 4, TCALC_RIGHT_ASSOC, tcalc_nequals_l));
  cleanup_on_err(err, tcalc_ctx_addbinlop(ctx, "||", 3, TCALC_RIGHT_ASSOC, tcalc_nequals_l));

  *out = ctx;
  return err;

  cleanup:
    tcalc_ctx_free(ctx);
    return err;
}

void tcalc_ctx_free(tcalc_ctx* ctx) {
  if (ctx == NULL) return;
  TCALC_VEC_FREE_F(ctx->binfuncs, tcalc_binfuncdef_free);
  TCALC_VEC_FREE_F(ctx->unfuncs, tcalc_unfuncdef_free);
  TCALC_VEC_FREE_F(ctx->vars, tcalc_vardef_free);
  TCALC_VEC_FREE_F(ctx->lvars, tcalc_lvardef_free);
  TCALC_VEC_FREE_F(ctx->unops, tcalc_unopdef_free);
  TCALC_VEC_FREE_F(ctx->binops, tcalc_binopdef_free);
  TCALC_VEC_FREE_F(ctx->relops, tcalc_relopdef_free);
  TCALC_VEC_FREE_F(ctx->unlops, tcalc_unlopdef_free);
  TCALC_VEC_FREE_F(ctx->binlops, tcalc_binlopdef_free);

  free(ctx);
}

#define tcalc_ctx_addxvar(vec, varname, varval, deftype, def_alloc, def_free) \
  for (size_t i = 0; i < vec.len; i++) { \
    if (strcmp(vec.arr[i]->id, varname) == 0) { \
      vec.arr[i]->val = varval; \
      return TCALC_OK; \
    } \
  } \
  deftype* def; \
  tcalc_err err = def_alloc(varname, varval, &def); \
  if (err) return err; \
  cleanup_on_macerr(err, TCALC_VEC_PUSH(vec, def, err)); \
  return TCALC_OK; \
  cleanup: \
    def_free(def); \
    return err; \


tcalc_err tcalc_ctx_addvar(tcalc_ctx* ctx, const char* name, double val) {
  tcalc_ctx_addxvar(ctx->vars, name, val, tcalc_vardef, tcalc_vardef_alloc, tcalc_vardef_free);
}

tcalc_err tcalc_ctx_addlvar(tcalc_ctx* ctx, const char* name, int val) {
  tcalc_ctx_addxvar(ctx->lvars, name, val, tcalc_lvardef, tcalc_lvardef_alloc, tcalc_lvardef_free);
}

#define tcalc_ctx_addxfunc(vec, funcname, funcptr, deftype, def_alloc, def_free) \
  for (size_t i = 0; i < vec.len; i++) { \
    if (strcmp(vec.arr[i]->id, funcname) == 0) { \
      vec.arr[i]->func = funcptr; \
      return TCALC_OK; \
    } \
  } \
  deftype* def; \
  tcalc_err err = def_alloc(funcname, funcptr, &def); \
  if (err) return err; \
  cleanup_on_macerr(err, TCALC_VEC_PUSH(vec, def, err)); \
  return TCALC_OK; \
  cleanup: \
    def_free(def); \
    return err;

tcalc_err tcalc_ctx_addunfunc(tcalc_ctx* ctx, const char* name, tcalc_unfunc func) {
  tcalc_ctx_addxfunc(ctx->unfuncs, name, func,
  tcalc_unfuncdef, tcalc_unfuncdef_alloc, tcalc_unfuncdef_free);
}

tcalc_err tcalc_ctx_addbinfunc(tcalc_ctx* ctx, const char* name, tcalc_binfunc func) {
  tcalc_ctx_addxfunc(ctx->binfuncs, name, func,
  tcalc_binfuncdef, tcalc_binfuncdef_alloc, tcalc_binfuncdef_free);
}

#define tcalc_ctx_addxop(vec, opid, prec_, assoc_, funcptr, deftype, def_alloc, def_free) \
  for (size_t i = 0; i < vec.len; i++) { \
    if (strcmp(vec.arr[i]->id, opid) == 0) { \
      vec.arr[i]->prec = prec_; \
      vec.arr[i]->assoc = assoc_; \
      vec.arr[i]->func = funcptr; \
      return TCALC_OK; \
    } \
  } \
  deftype* def; \
  tcalc_err err = def_alloc(opid, prec_, assoc_, funcptr, &def); \
  if (err) return err; \
  cleanup_on_macerr(err, TCALC_VEC_PUSH(vec, def, err)); \
  return TCALC_OK; \
  cleanup: \
    def_free(def); \
    return err;

tcalc_err tcalc_ctx_addunop(tcalc_ctx* ctx, const char* name, int prec, tcalc_assoc assoc, tcalc_unfunc func) {
  tcalc_ctx_addxop(ctx->unops, name, prec, assoc, func, tcalc_unopdef, tcalc_unopdef_alloc, tcalc_unopdef_free);
}

tcalc_err tcalc_ctx_addbinop(tcalc_ctx* ctx, const char* name, int prec, tcalc_assoc assoc, tcalc_binfunc func) {
  tcalc_ctx_addxop(ctx->binops, name, prec, assoc, func, tcalc_binopdef, tcalc_binopdef_alloc, tcalc_binopdef_free);
}

tcalc_err tcalc_ctx_addrelop(tcalc_ctx* ctx,const char* name, int prec, tcalc_assoc assoc, tcalc_relfunc func) {
  tcalc_ctx_addxop(ctx->relops, name, prec, assoc, func, tcalc_relopdef, tcalc_relopdef_alloc, tcalc_relopdef_free);
}

tcalc_err tcalc_ctx_addunlop(tcalc_ctx* ctx, const char* name, int prec, tcalc_assoc assoc, tcalc_unlfunc func) {
  tcalc_ctx_addxop(ctx->unlops, name, prec, assoc, func, tcalc_unlopdef, tcalc_unlopdef_alloc, tcalc_unlopdef_free);
}

tcalc_err tcalc_ctx_addbinlop(tcalc_ctx* ctx, const char* name, int prec, tcalc_assoc assoc, tcalc_binlfunc func) {
  tcalc_ctx_addxop(ctx->binlops, name, prec, assoc, func, tcalc_binlopdef, tcalc_binlopdef_alloc, tcalc_binlopdef_free);
}

#define tcalc_ctx_hasx(vec, idname) \
  for (size_t i = 0; i < vec.len; i++) { \
    if (strcmp(vec.arr[i]->id, idname) == 0) { \
      return 1; \
    } \
  } \
  return 0;

int tcalc_ctx_hasid(const tcalc_ctx* ctx, const char* name) {
  return tcalc_ctx_hasvar(ctx, name) || tcalc_ctx_haslvar(ctx, name) ||
  tcalc_ctx_hasfunc(ctx, name);
}

int tcalc_ctx_hasfunc(const tcalc_ctx* ctx, const char* name) {
  return tcalc_ctx_hasunfunc(ctx, name) || tcalc_ctx_hasbinfunc(ctx, name);
}

int tcalc_ctx_hasop(const tcalc_ctx* ctx, const char* name) {
  return tcalc_ctx_hasbinop(ctx, name) || tcalc_ctx_hasunop(ctx, name) ||
  tcalc_ctx_hasrelop(ctx, name) || tcalc_ctx_hasunlop(ctx, name) || 
  tcalc_ctx_hasbinlop(ctx, name);
}

int tcalc_ctx_hasunfunc(const tcalc_ctx* ctx, const char* name) {
  tcalc_ctx_hasx(ctx->unfuncs, name);
}

int tcalc_ctx_hasbinfunc(const tcalc_ctx* ctx, const char* name) {
  tcalc_ctx_hasx(ctx->binfuncs, name);
}

int tcalc_ctx_hasvar(const tcalc_ctx* ctx, const char* name) {
  tcalc_ctx_hasx(ctx->vars, name);
}

int tcalc_ctx_haslvar(const tcalc_ctx* ctx, const char* name) {
  tcalc_ctx_hasx(ctx->lvars, name);
}


int tcalc_ctx_hasunop(const tcalc_ctx* ctx, const char* name) {
  tcalc_ctx_hasx(ctx->unops, name);
}

int tcalc_ctx_hasbinop(const tcalc_ctx* ctx, const char* name) {
  tcalc_ctx_hasx(ctx->binops, name);
}

int tcalc_ctx_hasrelop(const tcalc_ctx* ctx, const char* name) {
  tcalc_ctx_hasx(ctx->relops, name);
}

int tcalc_ctx_hasunlop(const tcalc_ctx* ctx, const char* name) {
  tcalc_ctx_hasx(ctx->unlops, name);
}

int tcalc_ctx_hasbinlop(const tcalc_ctx* ctx, const char* name) {
  tcalc_ctx_hasx(ctx->binlops, name);
}

#define tcalc_ctx_getx(vec, idname, out) \
  for (size_t i = 0; i < vec.len; i++) { \
    if (strcmp(vec.arr[i]->id, idname) == 0) { \
      *out = vec.arr[i]; \
      return TCALC_OK; \
    } \
  } \
  return TCALC_NOT_FOUND;

tcalc_err tcalc_ctx_getvar(const tcalc_ctx* ctx, const char* name, tcalc_vardef** out) {
  tcalc_ctx_getx(ctx->vars, name, out);
}

tcalc_err tcalc_ctx_getlvar(const tcalc_ctx* ctx, const char* name, tcalc_lvardef** out) {
  tcalc_ctx_getx(ctx->lvars, name, out);
}

tcalc_err tcalc_ctx_getunfunc(const tcalc_ctx* ctx, const char* name, tcalc_unfuncdef** out) {
  tcalc_ctx_getx(ctx->unfuncs, name, out);
}

tcalc_err tcalc_ctx_getbinfunc(const tcalc_ctx* ctx, const char* name, tcalc_binfuncdef** out) {
  tcalc_ctx_getx(ctx->binfuncs, name, out);
}

tcalc_err tcalc_ctx_getunop(const tcalc_ctx* ctx, const char* name, tcalc_unopdef** out) {
  tcalc_ctx_getx(ctx->unops, name, out);
}

tcalc_err tcalc_ctx_getbinop(const tcalc_ctx* ctx, const char* name, tcalc_binopdef** out) {
  tcalc_ctx_getx(ctx->binops, name, out);
}

tcalc_err tcalc_ctx_getrelop(const tcalc_ctx* ctx, const char* name, tcalc_relopdef** out) {
  tcalc_ctx_getx(ctx->relops, name, out);
}

tcalc_err tcalc_ctx_getunlop(const tcalc_ctx* ctx, const char* name, tcalc_unlopdef** out) {
  tcalc_ctx_getx(ctx->unlops, name, out);
}

tcalc_err tcalc_ctx_getbinlop(const tcalc_ctx* ctx, const char* name, tcalc_binlopdef** out) {
  tcalc_ctx_getx(ctx->binlops, name, out);
}


#define tcalc_xvardef_alloc(optype, name, valval, out) \
  *out = NULL; \
  optype* def = (optype*)malloc(sizeof(optype)); \
  if (def == NULL) return TCALC_BAD_ALLOC; \
  tcalc_err err = tcalc_strdup(name, &def->id); \
  if (err) { \
    free(def); \
    return err; \
  } \
  def->val = valval; \
  *out = def; \
  return TCALC_OK;

tcalc_err tcalc_vardef_alloc(const char* name, double val, tcalc_vardef** out) {
  tcalc_xvardef_alloc(tcalc_vardef, name, val, out);
}

tcalc_err tcalc_lvardef_alloc(const char* name, int boolval, tcalc_lvardef** out) {
  tcalc_xvardef_alloc(tcalc_lvardef, name, boolval, out);
}

#define tcalc_xfuncdef_alloc(tcalctype, name, func, out) \
  *out = NULL; \
  tcalctype* def = (tcalctype*)malloc(sizeof(tcalctype)); \
  if (def == NULL) return TCALC_BAD_ALLOC; \
  tcalc_err err = tcalc_strdup(name, &def->id); \
  if (err) { \
    free(def); \
    return err; \
  } \
  def->func = func; \
  *out = def; \
  return TCALC_OK;

tcalc_err tcalc_binfuncdef_alloc(const char* name, tcalc_binfunc func, tcalc_binfuncdef** out) {
  tcalc_xfuncdef_alloc(tcalc_binfuncdef, name, func, out)
}

tcalc_err tcalc_unfuncdef_alloc(const char* name, tcalc_unfunc func, tcalc_unfuncdef** out) {
  tcalc_xfuncdef_alloc(tcalc_unfuncdef, name, func, out)
}

#define tcalc_xopdef_allocx(optype, name, prec, assoc, func, out) \
  *out = NULL; \
  optype* def = (optype*)malloc(sizeof(optype)); \
  if (def == NULL) return TCALC_BAD_ALLOC; \
  tcalc_err err = tcalc_strdup(name, &def->id); \
  if (err) { \
    free(def); \
    return err; \
  } \
  def->prec = prec; \
  def->assoc = assoc; \
  def->func = func; \
  *out = def; \
  return TCALC_OK;

tcalc_err tcalc_binopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_binfunc func, tcalc_binopdef** out) {
  tcalc_xopdef_allocx(tcalc_binopdef, name, prec, assoc, func, out)
}

tcalc_err tcalc_unopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_unfunc func, tcalc_unopdef** out) {
  tcalc_xopdef_allocx(tcalc_unopdef, name, prec, assoc, func, out)
}

tcalc_err tcalc_unlopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_unlfunc func, tcalc_unlopdef** out) {
  tcalc_xopdef_allocx(tcalc_unlopdef, name, prec, assoc, func, out)
}

tcalc_err tcalc_binlopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_binlfunc func, tcalc_binlopdef** out) {
  tcalc_xopdef_allocx(tcalc_binlopdef, name, prec, assoc, func, out)
}

tcalc_err tcalc_relopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_relfunc func, tcalc_relopdef** out) {
  tcalc_xopdef_allocx(tcalc_relopdef, name, prec, assoc, func, out)
}

#define tcalc_xdef_free(objname) \
  if (objname == NULL) return; \
  free(objname->id); \
  free(objname); \

void tcalc_vardef_free(tcalc_vardef* var_def) {
  tcalc_xdef_free(var_def);
}

void tcalc_lvardef_free(tcalc_lvardef* lvar_def) {
  tcalc_xdef_free(lvar_def)
}

void tcalc_binfuncdef_free(tcalc_binfuncdef* binary_func_def) {
  tcalc_xdef_free(binary_func_def);
}

void tcalc_unfuncdef_free(tcalc_unfuncdef* unary_func_def) {
  tcalc_xdef_free(unary_func_def);
}

void tcalc_binopdef_free(tcalc_binopdef* binary_op_def) {
  tcalc_xdef_free(binary_op_def);
}

void tcalc_unopdef_free(tcalc_unopdef* unary_op_def) {
  tcalc_xdef_free(unary_op_def);
}

void tcalc_relopdef_free(tcalc_relopdef* rel_op_def) {
  tcalc_xdef_free(rel_op_def);
}

void tcalc_unlopdef_free(tcalc_unlopdef* unl_op_def) {
  tcalc_xdef_free(unl_op_def);
}

void tcalc_binlopdef_free(tcalc_binlopdef* binl_op_def) {
  tcalc_xdef_free(binl_op_def);
}

#define tcalc_getxopdata(objname) \
  tcalc_opdata op_data; \
  op_data.prec = objname->prec; \
  op_data.assoc = objname->assoc; \
  return op_data; \

tcalc_opdata tcalc_getbinopdata(tcalc_binopdef* binary_op_def) {
  tcalc_getxopdata(binary_op_def);
}

tcalc_opdata tcalc_getunopdata(tcalc_unopdef* unary_op_def) {
  tcalc_getxopdata(unary_op_def);
}
