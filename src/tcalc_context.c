#include "tcalc_context.h"

#include "tcalc_error.h"
#include "tcalc_constants.h"
#include "tcalc_func.h"
#include "tcalc_mem.h"
#include "tcalc_string.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static tcalc_err tcalc_vardef_alloc(const char* name, tcalc_val val, tcalc_vardef** out);
static tcalc_err tcalc_binfuncdef_alloc(const char* name, tcalc_val_binfunc func, tcalc_binfuncdef** out);
static tcalc_err tcalc_unfuncdef_alloc(const char* name, tcalc_val_unfunc func, tcalc_unfuncdef** out);
static tcalc_err tcalc_binopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_val_binfunc func, tcalc_binopdef** out);
static tcalc_err tcalc_unopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_val_unfunc func, tcalc_unopdef** out);
static tcalc_err tcalc_relopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_val_relfunc func, tcalc_relopdef** out);
static tcalc_err tcalc_unlopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_val_unlfunc func, tcalc_unlopdef** out);
static tcalc_err tcalc_binlopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_val_binlfunc func, tcalc_binlopdef** out);

static void tcalc_vardef_free(tcalc_vardef* var_def);
static void tcalc_binfuncdef_free(tcalc_binfuncdef* binary_func_def);
static void tcalc_unfuncdef_free(tcalc_unfuncdef* unary_func_def);
static void tcalc_binopdef_free(tcalc_binopdef* binary_op_def);
static void tcalc_unopdef_free(tcalc_unopdef* unary_op_def);
static void tcalc_relopdef_free(tcalc_relopdef* rel_op_def);
static void tcalc_unlopdef_free(tcalc_unlopdef* unl_op_def);
static void tcalc_binlopdef_free(tcalc_binlopdef* binl_op_def);

tcalc_err tcalc_ctx_alloc_empty(tcalc_ctx** out) {
  // use of calloc is important here! We have to null all of the TCALC_VEC
  // structs inside the ctx.
  tcalc_ctx* ctx = (tcalc_ctx*)calloc(1, sizeof(tcalc_ctx));
  if (ctx == NULL) return TCALC_ERR_BAD_ALLOC;

  *out = ctx;
  return TCALC_ERR_OK;
}

tcalc_err tcalc_ctx_alloc_default(tcalc_ctx** out) {
  tcalc_ctx* ctx;
  tcalc_err err = tcalc_ctx_alloc_empty(&ctx);
  if (err) return err;

  // trigonometric functions
  cleanup_on_err(err, tcalc_ctx_addtrigrad(ctx));

  // Default Unary Functions:
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "log", tcalc_val_log));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "ln", tcalc_val_ln));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "exp", tcalc_val_exp));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "sqrt", tcalc_val_sqrt));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "cbrt", tcalc_val_cbrt));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "ceil", tcalc_val_ceil));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "floor", tcalc_val_floor));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "round", tcalc_val_round));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, "abs", tcalc_val_abs));

  // Default Binary Functions:
  cleanup_on_err(err, tcalc_ctx_addbinfunc(ctx, "pow", tcalc_val_pow));

  // Default Variables:
  cleanup_on_err(err, tcalc_ctx_addvar(ctx, "pi", TCALC_VAL_INIT_NUM(TCALC_PI)));
  cleanup_on_err(err, tcalc_ctx_addvar(ctx, "e", TCALC_VAL_INIT_NUM(TCALC_E)));

  // Default Logical Variables:
  cleanup_on_err(err, tcalc_ctx_addvar(ctx, "true", TCALC_VAL_INIT_BOOL(1)));
  cleanup_on_err(err, tcalc_ctx_addvar(ctx, "false", TCALC_VAL_INIT_BOOL(0)));

  // Default Logical Unary Operators:
  cleanup_on_err(err, tcalc_ctx_addunlop(ctx, "!", 10, TCALC_RIGHT_ASSOC, tcalc_val_not));

  // Default Unary Operators:
  cleanup_on_err(err, tcalc_ctx_addunop(ctx, "+", 10, TCALC_RIGHT_ASSOC, tcalc_val_unary_plus));
  cleanup_on_err(err, tcalc_ctx_addunop(ctx, "-", 10, TCALC_RIGHT_ASSOC, tcalc_val_unary_minus));

  // Default Binary Operators:
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "**", 10, TCALC_RIGHT_ASSOC, tcalc_val_pow));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "^", 10, TCALC_RIGHT_ASSOC, tcalc_val_pow));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "*", 9, TCALC_LEFT_ASSOC, tcalc_val_multiply));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "/", 9, TCALC_LEFT_ASSOC, tcalc_val_divide));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "%", 9, TCALC_LEFT_ASSOC, tcalc_val_mod));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "+", 8, TCALC_LEFT_ASSOC, tcalc_val_add));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "-", 8, TCALC_LEFT_ASSOC, tcalc_val_subtract));

  // Default Relational Binary Operators:
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, "<", 6, TCALC_LEFT_ASSOC, tcalc_val_lt));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, "<=", 6, TCALC_LEFT_ASSOC, tcalc_val_lteq));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, ">", 6, TCALC_LEFT_ASSOC, tcalc_val_gt));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, ">=", 6, TCALC_LEFT_ASSOC, tcalc_val_gteq));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, "=", 5, TCALC_RIGHT_ASSOC, tcalc_val_equals));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, "==", 5, TCALC_RIGHT_ASSOC, tcalc_val_equals));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, "!=", 5, TCALC_RIGHT_ASSOC, tcalc_val_nequals));



  // Default Logical Binary Operators:
  cleanup_on_err(err, tcalc_ctx_addbinlop(ctx, "=", 5, TCALC_RIGHT_ASSOC, tcalc_val_equals_l));
  cleanup_on_err(err, tcalc_ctx_addbinlop(ctx, "==", 5, TCALC_RIGHT_ASSOC, tcalc_val_equals_l));
  cleanup_on_err(err, tcalc_ctx_addbinlop(ctx, "!=", 5, TCALC_RIGHT_ASSOC, tcalc_val_nequals_l));
  cleanup_on_err(err, tcalc_ctx_addbinlop(ctx, "&&", 4, TCALC_RIGHT_ASSOC, tcalc_val_and));
  cleanup_on_err(err, tcalc_ctx_addbinlop(ctx, "||", 3, TCALC_RIGHT_ASSOC, tcalc_val_or));

  *out = ctx;
  return err;

  cleanup:
    tcalc_ctx_free(ctx);
    return err;
}

tcalc_err tcalc_ctx_addtrigrad(tcalc_ctx* ctx) {
  tcalc_err err = TCALC_ERR_OK;
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "sin", tcalc_val_sin));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "cos", tcalc_val_cos));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "tan", tcalc_val_tan));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "sec", tcalc_val_sec));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "csc", tcalc_val_csc));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "cot", tcalc_val_cot));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "asin", tcalc_val_asin));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arcsin", tcalc_val_asin));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "acos", tcalc_val_acos));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arccos", tcalc_val_acos));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "atan", tcalc_val_atan));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arctan", tcalc_val_atan));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "asec", tcalc_val_asec));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arcsec", tcalc_val_asec));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "acsc", tcalc_val_acsc));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arccsc", tcalc_val_acsc));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "acot", tcalc_val_acot));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arccot", tcalc_val_acot));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "sinh", tcalc_val_sinh));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "cosh", tcalc_val_cosh));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "tanh", tcalc_val_tanh));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "asinh", tcalc_val_asinh));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arcsinh", tcalc_val_asinh));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "acosh", tcalc_val_acosh));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arccosh", tcalc_val_acosh));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "atanh", tcalc_val_atanh));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arctanh", tcalc_val_atanh));
  ret_on_err(err, tcalc_ctx_addbinfunc(ctx, "atan2", tcalc_val_atan2));
  return err;
}

tcalc_err tcalc_ctx_addtrigdeg(tcalc_ctx* ctx) {
  tcalc_err err = TCALC_ERR_OK;
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "sin", tcalc_val_sin_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "cos", tcalc_val_cos_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "tan", tcalc_val_tan_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "sec", tcalc_val_sec_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "csc", tcalc_val_csc_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "cot", tcalc_val_cot_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "asin", tcalc_val_asin_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arcsin", tcalc_val_asin_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "acos", tcalc_val_acos_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arccos", tcalc_val_acos_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "atan", tcalc_val_atan_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arctan", tcalc_val_atan_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "asec", tcalc_val_asec_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arcsec", tcalc_val_asec_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "acsc", tcalc_val_acsc_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arccsc", tcalc_val_acsc_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "acot", tcalc_val_acot_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arccot", tcalc_val_acot_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "sinh", tcalc_val_sinh_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "cosh", tcalc_val_cosh_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "tanh", tcalc_val_tanh_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "asinh", tcalc_val_asinh_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arcsinh", tcalc_val_asinh_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "acosh", tcalc_val_acosh_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arccosh", tcalc_val_acosh_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "atanh", tcalc_val_atanh_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, "arctanh", tcalc_val_atanh_deg));
  ret_on_err(err, tcalc_ctx_addbinfunc(ctx, "atan2", tcalc_val_atan2_deg));
  return err;
}

void tcalc_ctx_free(tcalc_ctx* ctx) {
  if (ctx == NULL) return;
  TCALC_VEC_FREE_F(ctx->binfuncs, tcalc_binfuncdef_free);
  TCALC_VEC_FREE_F(ctx->unfuncs, tcalc_unfuncdef_free);
  TCALC_VEC_FREE_F(ctx->vars, tcalc_vardef_free);
  TCALC_VEC_FREE_F(ctx->unops, tcalc_unopdef_free);
  TCALC_VEC_FREE_F(ctx->binops, tcalc_binopdef_free);
  TCALC_VEC_FREE_F(ctx->relops, tcalc_relopdef_free);
  TCALC_VEC_FREE_F(ctx->unlops, tcalc_unlopdef_free);
  TCALC_VEC_FREE_F(ctx->binlops, tcalc_binlopdef_free);

  free(ctx);
}

tcalc_err tcalc_ctx_addvar(tcalc_ctx* ctx, const char* name, tcalc_val val) {
  for (size_t i = 0; i < ctx->vars.len; i++) {
    if (strcmp(ctx->vars.arr[i]->id, name) == 0) {
      ctx->vars.arr[i]->val = val;
      return TCALC_ERR_OK;
    }
  }

  tcalc_vardef* def;
  tcalc_err err = tcalc_vardef_alloc(name, val, &def);
  if (err) return err;

  cleanup_on_macerr(err, TCALC_VEC_PUSH(ctx->vars, def, err));
  return TCALC_ERR_OK;

  cleanup:
    tcalc_vardef_free(def);
    return err;
}

#define tcalc_ctx_addxfunc(vec, funcname, funcptr, deftype, def_alloc, def_free) \
  for (size_t i = 0; i < vec.len; i++) { \
    if (strcmp(vec.arr[i]->id, funcname) == 0) { \
      vec.arr[i]->func = funcptr; \
      return TCALC_ERR_OK; \
    } \
  } \
  deftype* def; \
  tcalc_err err = def_alloc(funcname, funcptr, &def); \
  if (err) return err; \
  cleanup_on_macerr(err, TCALC_VEC_PUSH(vec, def, err)); \
  return TCALC_ERR_OK; \
  cleanup: \
    def_free(def); \
    return err;

tcalc_err tcalc_ctx_addunfunc(tcalc_ctx* ctx, const char* name, tcalc_val_unfunc func) {
  tcalc_ctx_addxfunc(ctx->unfuncs, name, func,
  tcalc_unfuncdef, tcalc_unfuncdef_alloc, tcalc_unfuncdef_free);
}

tcalc_err tcalc_ctx_addbinfunc(tcalc_ctx* ctx, const char* name, tcalc_val_binfunc func) {
  tcalc_ctx_addxfunc(ctx->binfuncs, name, func,
  tcalc_binfuncdef, tcalc_binfuncdef_alloc, tcalc_binfuncdef_free);
}

#define tcalc_ctx_addxop(vec, opid, prec_, assoc_, funcptr, deftype, def_alloc, def_free) \
  for (size_t i = 0; i < vec.len; i++) { \
    if (strcmp(vec.arr[i]->id, opid) == 0) { \
      vec.arr[i]->prec = prec_; \
      vec.arr[i]->assoc = assoc_; \
      vec.arr[i]->func = funcptr; \
      return TCALC_ERR_OK; \
    } \
  } \
  deftype* def; \
  tcalc_err err = def_alloc(opid, prec_, assoc_, funcptr, &def); \
  if (err) return err; \
  cleanup_on_macerr(err, TCALC_VEC_PUSH(vec, def, err)); \
  return TCALC_ERR_OK; \
  cleanup: \
    def_free(def); \
    return err;

tcalc_err tcalc_ctx_addunop(tcalc_ctx* ctx, const char* name, int prec, tcalc_assoc assoc, tcalc_val_unfunc func) {
  tcalc_ctx_addxop(ctx->unops, name, prec, assoc, func, tcalc_unopdef, tcalc_unopdef_alloc, tcalc_unopdef_free);
}

tcalc_err tcalc_ctx_addbinop(tcalc_ctx* ctx, const char* name, int prec, tcalc_assoc assoc, tcalc_val_binfunc func) {
  tcalc_ctx_addxop(ctx->binops, name, prec, assoc, func, tcalc_binopdef, tcalc_binopdef_alloc, tcalc_binopdef_free);
}

tcalc_err tcalc_ctx_addrelop(tcalc_ctx* ctx,const char* name, int prec, tcalc_assoc assoc, tcalc_val_relfunc func) {
  tcalc_ctx_addxop(ctx->relops, name, prec, assoc, func, tcalc_relopdef, tcalc_relopdef_alloc, tcalc_relopdef_free);
}

tcalc_err tcalc_ctx_addunlop(tcalc_ctx* ctx, const char* name, int prec, tcalc_assoc assoc, tcalc_val_unlfunc func) {
  tcalc_ctx_addxop(ctx->unlops, name, prec, assoc, func, tcalc_unlopdef, tcalc_unlopdef_alloc, tcalc_unlopdef_free);
}

tcalc_err tcalc_ctx_addbinlop(tcalc_ctx* ctx, const char* name, int prec, tcalc_assoc assoc, tcalc_val_binlfunc func) {
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
  return tcalc_ctx_hasvar(ctx, name) || tcalc_ctx_hasfunc(ctx, name);
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
      return TCALC_ERR_OK; \
    } \
  } \
  return TCALC_ERR_NOT_FOUND;

tcalc_err tcalc_ctx_getvar(const tcalc_ctx* ctx, const char* name, tcalc_vardef** out) {
  tcalc_ctx_getx(ctx->vars, name, out);
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


static tcalc_err tcalc_vardef_alloc(const char* name, tcalc_val val, tcalc_vardef** out) {
  *out = NULL;
  tcalc_vardef* def = (tcalc_vardef*)malloc(sizeof(tcalc_vardef));
  if (def == NULL) return TCALC_ERR_BAD_ALLOC;

  char* dupstr = NULL;
  tcalc_err err = tcalc_strdup(name, &dupstr);
  if (err) {
    free(def);
    return err;
  }

  def->id = dupstr;
  def->val = val;
  *out = def;
  return TCALC_ERR_OK;
}

#define tcalc_xfuncdef_alloc(tcalctype, name, func, out) \
  *out = NULL; \
  char* dupstr = NULL; \
  tcalctype* def = (tcalctype*)malloc(sizeof(tcalctype)); \
  if (def == NULL) return TCALC_ERR_BAD_ALLOC; \
  tcalc_err err = tcalc_strdup(name, &dupstr); \
  if (err) { \
    free(def); \
    return err; \
  } \
  def->id = dupstr; \
  def->func = func; \
  *out = def; \
  return TCALC_ERR_OK;

static tcalc_err tcalc_binfuncdef_alloc(const char* name, tcalc_val_binfunc func, tcalc_binfuncdef** out) {
  tcalc_xfuncdef_alloc(tcalc_binfuncdef, name, func, out)
}

static tcalc_err tcalc_unfuncdef_alloc(const char* name, tcalc_val_unfunc func, tcalc_unfuncdef** out) {
  tcalc_xfuncdef_alloc(tcalc_unfuncdef, name, func, out)
}

#define tcalc_xopdef_allocx(optype, name, prec, assoc, func, out) \
  *out = NULL; \
  char* dupstr = NULL; \
  optype* def = (optype*)malloc(sizeof(optype)); \
  if (def == NULL) return TCALC_ERR_BAD_ALLOC; \
  tcalc_err err = tcalc_strdup(name, &dupstr); \
  if (err) { \
    free(def); \
    return err; \
  } \
  def->id = dupstr; \
  def->prec = prec; \
  def->assoc = assoc; \
  def->func = func; \
  *out = def; \
  return TCALC_ERR_OK;

static tcalc_err tcalc_binopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_val_binfunc func, tcalc_binopdef** out) {
  tcalc_xopdef_allocx(tcalc_binopdef, name, prec, assoc, func, out)
}

static tcalc_err tcalc_unopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_val_unfunc func, tcalc_unopdef** out) {
  tcalc_xopdef_allocx(tcalc_unopdef, name, prec, assoc, func, out)
}

static tcalc_err tcalc_unlopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_val_unlfunc func, tcalc_unlopdef** out) {
  tcalc_xopdef_allocx(tcalc_unlopdef, name, prec, assoc, func, out)
}

static tcalc_err tcalc_binlopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_val_binlfunc func, tcalc_binlopdef** out) {
  tcalc_xopdef_allocx(tcalc_binlopdef, name, prec, assoc, func, out)
}

static tcalc_err tcalc_relopdef_alloc(const char* name, int prec, tcalc_assoc assoc, tcalc_val_relfunc func, tcalc_relopdef** out) {
  tcalc_xopdef_allocx(tcalc_relopdef, name, prec, assoc, func, out)
}

#define tcalc_xdef_free(objname) \
  if (objname == NULL) return; \
  free((void*)objname->id); \
  free(objname); \

static void tcalc_vardef_free(tcalc_vardef* var_def) {
  tcalc_xdef_free(var_def);
}

static void tcalc_binfuncdef_free(tcalc_binfuncdef* binary_func_def) {
  tcalc_xdef_free(binary_func_def);
}

static void tcalc_unfuncdef_free(tcalc_unfuncdef* unary_func_def) {
  tcalc_xdef_free(unary_func_def);
}

static void tcalc_binopdef_free(tcalc_binopdef* binary_op_def) {
  tcalc_xdef_free(binary_op_def);
}

static void tcalc_unopdef_free(tcalc_unopdef* unary_op_def) {
  tcalc_xdef_free(unary_op_def);
}

static void tcalc_relopdef_free(tcalc_relopdef* rel_op_def) {
  tcalc_xdef_free(rel_op_def);
}

static void tcalc_unlopdef_free(tcalc_unlopdef* unl_op_def) {
  tcalc_xdef_free(unl_op_def);
}

static void tcalc_binlopdef_free(tcalc_binlopdef* binl_op_def) {
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

tcalc_opdata tcalc_getrelopdata(tcalc_relopdef* relop_def) {
  tcalc_getxopdata(relop_def);
}

tcalc_opdata tcalc_getunlopdata(tcalc_unlopdef* unlop_def) {
  tcalc_getxopdata(unlop_def);
}

tcalc_opdata tcalc_getbinlopdata(tcalc_binlopdef* binlop_def) {
  tcalc_getxopdata(binlop_def);
}
