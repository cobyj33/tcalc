#include "tcalc_context.h"

#include "tcalc_constants.h"
#include "tcalc_func.h"
#include "tcalc_mem.h"
#include "tcalc_string.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

tcalc_err tcalc_vardef_alloc(char* name, double value, tcalc_vardef** out);
tcalc_err tcalc_binfuncdef_alloc(char* name, tcalc_binfunc func, tcalc_binfuncdef** out);
tcalc_err tcalc_unfuncdef_alloc(char* name, tcalc_unfunc func, tcalc_unfuncdef** out);
tcalc_err tcalc_binopdef_alloc(char* name, int prec, tcalc_assoc assoc, tcalc_binfunc func, tcalc_binopdef** out);
tcalc_err tcalc_unopdef_alloc(char* name, int prec, tcalc_assoc assoc, tcalc_unfunc func, tcalc_uopdef** out);

void tcalc_vardef_free(tcalc_vardef* var_def);
void tcalc_binfuncdef_free(tcalc_binfuncdef* binary_func_def);
void tcalc_unfuncdef_free(tcalc_unfuncdef* unary_func_def);
void tcalc_binopdef_free(tcalc_binopdef* binary_op_def);
void tcalc_unopdef_free(tcalc_uopdef* unary_op_def);

tcalc_err tcalc_ctx_alloc_empty(tcalc_ctx** out) {
  tcalc_ctx* context = (tcalc_ctx*)calloc(1, sizeof(tcalc_ctx)); // use of calloc is important here
  if (context == NULL) return TCALC_BAD_ALLOC;

  *out = context;
  return TCALC_OK; 
}

tcalc_err tcalc_ctx_alloc_default(tcalc_ctx** out) {
  tcalc_ctx* context;
  tcalc_err err = tcalc_ctx_alloc_empty(&context);
  if (err) return err;

  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "sin", tcalc_sin));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "cos", tcalc_cos));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "tan", tcalc_tan));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "sec", tcalc_sec));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "csc", tcalc_csc));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "cot", tcalc_cot));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "asin", tcalc_asin));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "arcsin", tcalc_asin));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "acos", tcalc_acos));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "arccos", tcalc_acos));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "atan", tcalc_atan));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "arctan", tcalc_atan));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "asec", tcalc_asec));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "arcsec", tcalc_asec));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "acsc", tcalc_acsc));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "arccsc", tcalc_acsc));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "acot", tcalc_acot));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "arccot", tcalc_acot));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "sinh", tcalc_sinh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "cosh", tcalc_cosh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "tanh", tcalc_tanh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "asinh", tcalc_asinh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "arcsinh", tcalc_asinh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "acosh", tcalc_acosh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "arccosh", tcalc_acosh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "atanh", tcalc_atanh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "arctanh", tcalc_atanh));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "log", tcalc_log));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "ln", tcalc_ln));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "exp", tcalc_exp));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "sqrt", tcalc_sqrt));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "cbrt", tcalc_cbrt));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "ceil", tcalc_ceil));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "floor", tcalc_floor));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "round", tcalc_round));
  cleanup_on_err(err, tcalc_ctx_addunfunc(context, "abs", tcalc_abs));

  cleanup_on_err(err, tcalc_ctx_addbinfunc(context, "pow", tcalc_pow));


  cleanup_on_err(err, tcalc_ctx_addvar(context, "pi", M_PI));
  cleanup_on_err(err, tcalc_ctx_addvar(context, "e", M_E));

  cleanup_on_err(err, tcalc_ctx_addunop(context, "+", 3, TCALC_RIGHT_ASSOC, tcalc_unary_plus));
  cleanup_on_err(err, tcalc_ctx_addunop(context, "-", 3, TCALC_RIGHT_ASSOC, tcalc_unary_minus));

  cleanup_on_err(err, tcalc_ctx_addbinop(context, "+", 1, TCALC_LEFT_ASSOC, tcalc_add));
  cleanup_on_err(err, tcalc_ctx_addbinop(context, "-", 1, TCALC_LEFT_ASSOC, tcalc_subtract));
  cleanup_on_err(err, tcalc_ctx_addbinop(context, "*", 2, TCALC_LEFT_ASSOC, tcalc_multiply));
  cleanup_on_err(err, tcalc_ctx_addbinop(context, "/", 2, TCALC_LEFT_ASSOC, tcalc_divide));
  cleanup_on_err(err, tcalc_ctx_addbinop(context, "%", 2, TCALC_LEFT_ASSOC, tcalc_mod));
  cleanup_on_err(err, tcalc_ctx_addbinop(context, "^", 3, TCALC_RIGHT_ASSOC, tcalc_pow));

  *out = context;
  return err;

  cleanup:
    tcalc_ctx_free(context);
    return err;
}

void tcalc_ctx_free(tcalc_ctx* context) {
  TCALC_VEC_FREE_F(context->binfuncs, tcalc_binfuncdef_free);
  TCALC_VEC_FREE_F(context->unfuncs, tcalc_unfuncdef_free);
  TCALC_VEC_FREE_F(context->vars, tcalc_vardef_free);
  TCALC_VEC_FREE_F(context->unops, tcalc_unopdef_free);
  TCALC_VEC_FREE_F(context->binops, tcalc_binopdef_free);

  free(context);
}

tcalc_err tcalc_ctx_addvar(tcalc_ctx* context, char* name, double value) {
  for (size_t i = 0; i < context->vars.len; i++) {
    if (strcmp(context->vars.arr[i]->id, name) == 0) {
      context->vars.arr[i]->value = value;
      return TCALC_OK;
    }
  }

  tcalc_vardef* variable;
  tcalc_err err = tcalc_vardef_alloc(name, value, &variable);
  if (err) return err;

  TCALC_VEC_PUSH(context->vars, variable, err);
  if (err) goto cleanup;

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

  TCALC_VEC_PUSH(ctx->unfuncs, unary_func, err);
  if (err) goto cleanup;

  return TCALC_OK;
  cleanup:
    tcalc_unfuncdef_free(unary_func);
    return err;
}

tcalc_err tcalc_ctx_addbinfunc(tcalc_ctx* ctx, char* name, tcalc_binfunc func) {
  tcalc_binfuncdef* binary_func;
  tcalc_err err = tcalc_binfuncdef_alloc(name, func, &binary_func);
  if (err) return err;

  TCALC_VEC_PUSH(ctx->binfuncs, binary_func, err);
  if (err) goto cleanup;

  return TCALC_OK;
  cleanup:
    tcalc_binfuncdef_free(binary_func);
    return err;
}

tcalc_err tcalc_ctx_addunop(tcalc_ctx* ctx, char* name, int prec, tcalc_assoc assoc, tcalc_unfunc func) {
  tcalc_uopdef* unary_op;
  tcalc_err err = tcalc_unopdef_alloc(name, prec, assoc, func, &unary_op);
  if (err) return err;

  TCALC_VEC_PUSH(ctx->unops, unary_op, err);
  if (err) goto cleanup;

  return TCALC_OK;
  cleanup:
    tcalc_unopdef_free(unary_op);
    return err;
}

tcalc_err tcalc_ctx_addbinop(tcalc_ctx* context,  char* name, int prec, tcalc_assoc assoc, tcalc_binfunc func) {
  tcalc_binopdef* binary_op;
  tcalc_err err = tcalc_binopdef_alloc(name, prec, assoc, func, &binary_op);
  if (err) return err;
  TCALC_VEC_PUSH(context->binops, binary_op, err);
  if (err) goto cleanup;

  return TCALC_OK;
  cleanup:
    tcalc_binopdef_free(binary_op);
    return err;
}

int tcalc_ctx_hasid(const tcalc_ctx* context, const char* name) {
  return tcalc_ctx_hasvar(context, name) ||
  tcalc_ctx_hasfunc(context, name);
}

int tcalc_ctx_hasfunc(const tcalc_ctx* context, const char* name) {
  return tcalc_ctx_hasunfunc(context, name) ||
  tcalc_ctx_hasbinfunc(context, name);
}

int tcalc_ctx_hasunfunc(const tcalc_ctx* context, const char* name) {
  return tcalc_ctx_getunfunc(context, name, NULL) == TCALC_OK;
}

int tcalc_ctx_hasbinfunc(const tcalc_ctx* context, const char* name) {
  return tcalc_ctx_getbinfunc(context, name, NULL) == TCALC_OK;
}

int tcalc_ctx_hasvar(const tcalc_ctx* context, const char* name) {
  return tcalc_ctx_getvar(context, name, NULL) == TCALC_OK;
}

int tcalc_ctx_hasunop(const tcalc_ctx* context, const char* name) {
  return tcalc_ctx_getunop(context, name, NULL) == TCALC_OK;
}

int tcalc_ctx_hasbinop(const tcalc_ctx* context, const char* name) {
  return tcalc_ctx_getbinop(context, name, NULL) == TCALC_OK;
}

int tcalc_ctx_hasop(const tcalc_ctx* context, const char* name) {
  return tcalc_ctx_hasbinop(context, name) || tcalc_ctx_hasunop(context, name);
}

tcalc_err tcalc_ctx_getunfunc(const tcalc_ctx* context, const char* name, tcalc_unfuncdef** out) {
  for (size_t i = 0; i < context->unfuncs.len; i++) {
    if (strcmp(context->unfuncs.arr[i]->id, name) == 0) {
      if (out != NULL) *out = context->unfuncs.arr[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

tcalc_err tcalc_ctx_getbinfunc(const tcalc_ctx* context, const char* name, tcalc_binfuncdef** out) {
  for (size_t i = 0; i < context->binfuncs.len; i++) {
    if (strcmp(context->binfuncs.arr[i]->id, name) == 0) {
      if (out != NULL) *out = context->binfuncs.arr[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

tcalc_err tcalc_ctx_getvar(const tcalc_ctx* context, const char* name, tcalc_vardef** out) {
  for (size_t i = 0; i < context->vars.len; i++) {
    if (strcmp(context->vars.arr[i]->id, name) == 0) {
      if (out != NULL) *out = context->vars.arr[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

tcalc_err tcalc_ctx_getunop(const tcalc_ctx* context, const char* name, tcalc_uopdef** out) {
  for (size_t i = 0; i < context->unops.len; i++) {
    if (strcmp(context->unops.arr[i]->id, name) == 0) {
      if (out != NULL) *out = context->unops.arr[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

tcalc_err tcalc_ctx_getbinop(const tcalc_ctx* context, const char* name, tcalc_binopdef** out) {
  for (size_t i = 0; i < context->binops.len; i++) {
    if (strcmp(context->binops.arr[i]->id, name) == 0) {
      if (out != NULL) *out = context->binops.arr[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

tcalc_err tcalc_vardef_alloc(char* name, double value, tcalc_vardef** out) {
  tcalc_vardef* var_def = (tcalc_vardef*)malloc(sizeof(tcalc_vardef));
  if (var_def == NULL) return TCALC_BAD_ALLOC;

  tcalc_err err = tcalc_strdup(name, &var_def->id);
  if (err) {
    free(var_def);
    return err;
  }

  var_def->value = value;
  *out = var_def;
  return TCALC_OK;
}

tcalc_err tcalc_binfuncdef_alloc(char* name, tcalc_binfunc func, tcalc_binfuncdef** out) {
  tcalc_binfuncdef* binary_func = (tcalc_binfuncdef*)malloc(sizeof(tcalc_binfuncdef));
  if (binary_func == NULL) return TCALC_BAD_ALLOC;

  tcalc_err err = tcalc_strdup(name, &binary_func->id);
  if (err) {
    free(binary_func);
    return err;
  }

  binary_func->func = func;
  *out = binary_func;
  return TCALC_OK;
}

tcalc_err tcalc_unfuncdef_alloc(char* name, tcalc_unfunc func, tcalc_unfuncdef** out) {
  tcalc_unfuncdef* unary_func = (tcalc_unfuncdef*)malloc(sizeof(tcalc_unfuncdef));
  if (unary_func == NULL) return TCALC_BAD_ALLOC;

  tcalc_err err = tcalc_strdup(name, &unary_func->id);
  if (err) {
    free(unary_func);
    return err;
  }

  unary_func->func = func;
  *out = unary_func;
  return TCALC_OK;
}

tcalc_err tcalc_binopdef_alloc(char* name, int prec, tcalc_assoc assoc, tcalc_binfunc func, tcalc_binopdef** out) {
  tcalc_binopdef* binary_op_def = (tcalc_binopdef*)malloc(sizeof(tcalc_binopdef));
  if (binary_op_def == NULL) return TCALC_BAD_ALLOC;

  tcalc_err err = tcalc_strdup(name, &binary_op_def->id);
  if (err) {
    free(binary_op_def);
    return err;
  }

  binary_op_def->prec = prec;
  binary_op_def->assoc = assoc;
  binary_op_def->func = func;
  *out = binary_op_def;
  return TCALC_OK;
}

tcalc_err tcalc_unopdef_alloc(char* name, int prec, tcalc_assoc assoc, tcalc_unfunc func, tcalc_uopdef** out) {
  tcalc_uopdef* unary_op_def = (tcalc_uopdef*)malloc(sizeof(tcalc_uopdef));
  if (unary_op_def == NULL) return TCALC_BAD_ALLOC;

  tcalc_err err = tcalc_strdup(name, &unary_op_def->id);
  if (err) {
    free(unary_op_def);
    return err;
  }

  unary_op_def->prec = prec;
  unary_op_def->assoc = assoc;
  unary_op_def->func = func;
  *out = unary_op_def;
  return TCALC_OK;
}

void tcalc_vardef_free(tcalc_vardef* var_def) {
  if (var_def == NULL) return;
  free(var_def->id);
  free(var_def);
}

void tcalc_binfuncdef_free(tcalc_binfuncdef* binary_func_def) {
  if (binary_func_def == NULL) return;
  free(binary_func_def->id);
  free(binary_func_def);
}

void tcalc_unfuncdef_free(tcalc_unfuncdef* unary_func_def) {
  if (unary_func_def == NULL) return;
  free(unary_func_def->id);
  free(unary_func_def);
}

void tcalc_binopdef_free(tcalc_binopdef* binary_op_def) {
  if (binary_op_def == NULL) return;
  free(binary_op_def->id);
  free(binary_op_def);
}

void tcalc_unopdef_free(tcalc_uopdef* unary_op_def) {
  if (unary_op_def == NULL) return;
  free(unary_op_def->id);
  free(unary_op_def);
}

tcalc_opdata tcalc_getbinopdata(tcalc_binopdef* binary_op_def) {
  tcalc_opdata op_data;
  op_data.prec = binary_op_def->prec;
  op_data.assoc = binary_op_def->assoc;
  return op_data;
}

tcalc_opdata tcalc_getunopdata(tcalc_uopdef* unary_op_def) {
  tcalc_opdata op_data;
  op_data.prec = unary_op_def->prec;
  op_data.assoc = unary_op_def->assoc;
  return op_data;
}