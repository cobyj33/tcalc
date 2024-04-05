#include "tcalc_context.h"

#include "tcalc_error.h"
#include "tcalc_constants.h"
#include "tcalc_func.h"
#include "tcalc_mem.h"
#include "tcalc_string.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

tcalc_err tcalc_vardef_alloc(char* name, double val, tcalc_vardef** out);
tcalc_err tcalc_varldef_alloc(char* name, int boolval, tcalc_varldef** out);
tcalc_err tcalc_binfuncdef_alloc(char* name, tcalc_binfunc func, tcalc_binfuncdef** out);
tcalc_err tcalc_unfuncdef_alloc(char* name, tcalc_unfunc func, tcalc_unfuncdef** out);
tcalc_err tcalc_binopdef_alloc(char* name, int prec, tcalc_assoc assoc, tcalc_binfunc func, tcalc_binopdef** out);
tcalc_err tcalc_unopdef_alloc(char* name, int prec, tcalc_assoc assoc, tcalc_unfunc func, tcalc_unopdef** out);
tcalc_err tcalc_unlopdef_alloc(char* name, int prec, tcalc_assoc assoc, tcalc_unlfunc func, tcalc_unlopdef** out);
tcalc_err tcalc_binlopdef_alloc(char* name, int prec, tcalc_assoc assoc, tcalc_binlfunc func, tcalc_binlopdef** out);

void tcalc_vardef_free(tcalc_vardef* var_def);
void tcalc_binfuncdef_free(tcalc_binfuncdef* binary_func_def);
void tcalc_unfuncdef_free(tcalc_unfuncdef* unary_func_def);
void tcalc_binopdef_free(tcalc_binopdef* binary_op_def);
void tcalc_unopdef_free(tcalc_unopdef* unary_op_def);

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

  // Default Unary Operators:
  cleanup_on_err(err, tcalc_ctx_addunop(ctx, "+", 3, TCALC_RIGHT_ASSOC, tcalc_unary_plus));
  cleanup_on_err(err, tcalc_ctx_addunop(ctx, "-", 3, TCALC_RIGHT_ASSOC, tcalc_unary_minus));

  // Default Binary Operators:
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "+", 1, TCALC_LEFT_ASSOC, tcalc_add));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "-", 1, TCALC_LEFT_ASSOC, tcalc_subtract));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "*", 2, TCALC_LEFT_ASSOC, tcalc_multiply));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "/", 2, TCALC_LEFT_ASSOC, tcalc_divide));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "%", 2, TCALC_LEFT_ASSOC, tcalc_mod));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "^", 3, TCALC_RIGHT_ASSOC, tcalc_pow));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "**", 3, TCALC_RIGHT_ASSOC, tcalc_pow));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, "**", 3, TCALC_RIGHT_ASSOC, tcalc_pow));

  // Default Relational Binary Operators:

  // Default Logical Unary Operators:

  // Default Logical Binary Operators:

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
  TCALC_VEC_FREE_F(ctx->unops, tcalc_unopdef_free);
  TCALC_VEC_FREE_F(ctx->binops, tcalc_binopdef_free);

  free(ctx);
}


tcalc_err tcalc_ctx_addvar(tcalc_ctx* ctx, char* name, double val) {
  for (size_t i = 0; i < ctx->vars.len; i++) {
    if (strcmp(ctx->vars.arr[i]->id, name) == 0) {
      ctx->vars.arr[i]->val = val;
      return TCALC_OK;
    }
  }
  tcalc_vardef* variable;
  tcalc_err err = tcalc_vardef_alloc(name, val, &variable);
  if (err) return err;
  cleanup_on_macerr(err, TCALC_VEC_PUSH(ctx->vars, variable, err));

  return TCALC_OK;
  cleanup:
    tcalc_vardef_free(variable);
    return err;
}

tcalc_err tcalc_ctx_addunfunc(tcalc_ctx* ctx, char* name, tcalc_unfunc func) {
  for (size_t i = 0; i < ctx->unfuncs.len; i++) {
    if (strcmp(ctx->unfuncs.arr[i]->id, name) == 0) {
      ctx->unfuncs.arr[i]->func = func;
      return TCALC_OK;
    }
  }

  tcalc_unfuncdef* unary_func;
  tcalc_err err = tcalc_unfuncdef_alloc(name, func, &unary_func);
  if (err) return err;

  cleanup_on_macerr(err, TCALC_VEC_PUSH(ctx->unfuncs, unary_func, err));

  return TCALC_OK;
  cleanup:
    tcalc_unfuncdef_free(unary_func);
    return err;
}

tcalc_err tcalc_ctx_addbinfunc(tcalc_ctx* ctx, char* name, tcalc_binfunc func) {
  tcalc_binfuncdef* binary_func;
  tcalc_err err = tcalc_binfuncdef_alloc(name, func, &binary_func);
  if (err) return err;

  cleanup_on_macerr(err, TCALC_VEC_PUSH(ctx->binfuncs, binary_func, err));

  return TCALC_OK;
  cleanup:
    tcalc_binfuncdef_free(binary_func);
    return err;
}

tcalc_err tcalc_ctx_addunop(tcalc_ctx* ctx, char* name, int prec, tcalc_assoc assoc, tcalc_unfunc func) {
  tcalc_unopdef* unary_op;
  tcalc_err err = tcalc_unopdef_alloc(name, prec, assoc, func, &unary_op);
  if (err) return err;

  cleanup_on_macerr(err, TCALC_VEC_PUSH(ctx->unops, unary_op, err));

  return TCALC_OK;
  cleanup:
    tcalc_unopdef_free(unary_op);
    return err;
}

tcalc_err tcalc_ctx_addbinop(tcalc_ctx* ctx,  char* name, int prec, tcalc_assoc assoc, tcalc_binfunc func) {
  tcalc_binopdef* binary_op;
  tcalc_err err = tcalc_binopdef_alloc(name, prec, assoc, func, &binary_op);
  if (err) return err;
  
  cleanup_on_macerr(err, TCALC_VEC_PUSH(ctx->binops, binary_op, err));

  return TCALC_OK;
  cleanup:
    tcalc_binopdef_free(binary_op);
    return err;
}

#define tcalc_ctx_hasx(ctx, arrname, idname) \
  for (size_t i = 0; i < ctx->arrname.len; i++) { \
    if (strcmp(ctx->arrname.arr[i]->id, idname) == 0) { \
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
  return tcalc_ctx_hasbinop(ctx, name) || tcalc_ctx_hasunop(ctx, name);
}

int tcalc_ctx_hasunfunc(const tcalc_ctx* ctx, const char* name) {
  tcalc_ctx_hasx(ctx, unfuncs, name);
}

int tcalc_ctx_hasbinfunc(const tcalc_ctx* ctx, const char* name) {
  tcalc_ctx_hasx(ctx, binfuncs, name);
}

int tcalc_ctx_hasvar(const tcalc_ctx* ctx, const char* name) {
  tcalc_ctx_hasx(ctx, vars, name);
}

int tcalc_ctx_hasunop(const tcalc_ctx* ctx, const char* name) {
  tcalc_ctx_hasx(ctx, unops, name);
}

int tcalc_ctx_hasbinop(const tcalc_ctx* ctx, const char* name) {
  tcalc_ctx_hasx(ctx, binops, name);
}

#define tcalc_ctx_getx(ctx, arrname, idname, out) \
  for (size_t i = 0; i < ctx->arrname.len; i++) { \
    if (strcmp(ctx->arrname.arr[i]->id, idname) == 0) { \
      *out = ctx->arrname.arr[i]; \
      return TCALC_OK; \
    } \
  } \
  return TCALC_NOT_FOUND;

tcalc_err tcalc_ctx_getunfunc(const tcalc_ctx* ctx, const char* name, tcalc_unfuncdef** out) {
  tcalc_ctx_getx(ctx, unfuncs, name, out);
}

tcalc_err tcalc_ctx_getbinfunc(const tcalc_ctx* ctx, const char* name, tcalc_binfuncdef** out) {
  tcalc_ctx_getx(ctx, binfuncs, name, out);
}

tcalc_err tcalc_ctx_getvar(const tcalc_ctx* ctx, const char* name, tcalc_vardef** out) {
  tcalc_ctx_getx(ctx, vars, name, out);
}

tcalc_err tcalc_ctx_getunop(const tcalc_ctx* ctx, const char* name, tcalc_unopdef** out) {
  tcalc_ctx_getx(ctx, unops, name, out);
}

tcalc_err tcalc_ctx_getbinop(const tcalc_ctx* ctx, const char* name, tcalc_binopdef** out) {
  tcalc_ctx_getx(ctx, binops, name, out);
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

tcalc_err tcalc_vardef_alloc(char* name, double val, tcalc_vardef** out) {
  tcalc_xvardef_alloc(tcalc_vardef, name, val, out);
}

tcalc_err tcalc_varldef_alloc(char* name, int boolval, tcalc_varldef** out) {
  tcalc_xvardef_alloc(tcalc_varldef, name, boolval, out);
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

tcalc_err tcalc_binfuncdef_alloc(char* name, tcalc_binfunc func, tcalc_binfuncdef** out) {
  tcalc_xfuncdef_alloc(tcalc_binfuncdef, name, func, out)
}

tcalc_err tcalc_unfuncdef_alloc(char* name, tcalc_unfunc func, tcalc_unfuncdef** out) {
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

tcalc_err tcalc_binopdef_alloc(char* name, int prec, tcalc_assoc assoc, tcalc_binfunc func, tcalc_binopdef** out) {
  tcalc_xopdef_allocx(tcalc_binopdef, name, prec, assoc, func, out)
}

tcalc_err tcalc_unopdef_alloc(char* name, int prec, tcalc_assoc assoc, tcalc_unfunc func, tcalc_unopdef** out) {
  tcalc_xopdef_allocx(tcalc_unopdef, name, prec, assoc, func, out)
}

tcalc_err tcalc_unlopdef_alloc(char* name, int prec, tcalc_assoc assoc, tcalc_unlfunc func, tcalc_unlopdef** out) {
  tcalc_xopdef_allocx(tcalc_unlopdef, name, prec, assoc, func, out)
}

tcalc_err tcalc_binlopdef_alloc(char* name, int prec, tcalc_assoc assoc, tcalc_binlfunc func, tcalc_binlopdef** out) {
  tcalc_xopdef_allocx(tcalc_binlopdef, name, prec, assoc, func, out)
}

#define tcalc_xdef_free(objname) \
  if (objname == NULL) return; \
  free(objname->id); \
  free(objname); \

void tcalc_vardef_free(tcalc_vardef* var_def) {
  tcalc_xdef_free(var_def);
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