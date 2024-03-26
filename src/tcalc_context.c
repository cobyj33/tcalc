#include "tcalc_context.h"

#include "tcalc_constants.h"
#include "tcalc_func.h"
#include "tcalc_mem.h"
#include "tcalc_string.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

tcalc_err tcalc_variable_def_alloc(char* name, double value, tcalc_vardef** out);
tcalc_err tcalc_binary_func_def_alloc(char* name, tcalc_binfunc function, tcalc_binfuncdef** out);
tcalc_err tcalc_unary_func_def_alloc(char* name, tcalc_unfunc function, tcalc_unfuncdef** out);
tcalc_err tcalc_binary_op_def_alloc(char* name, int precedence, tcalc_assoc associativity, tcalc_binfunc function, tcalc_binopdef** out);
tcalc_err tcalc_unary_op_def_alloc(char* name, int precedence, tcalc_assoc associativity, tcalc_unfunc function, tcalc_uopdef** out);

void tcalc_variable_def_free(tcalc_vardef* var_def);
void tcalc_binary_func_def_free(tcalc_binfuncdef* binary_func_def);
void tcalc_unary_func_def_free(tcalc_unfuncdef* unary_func_def);
void tcalc_binary_op_def_free(tcalc_binopdef* binary_op_def);
void tcalc_unary_op_def_free(tcalc_uopdef* unary_op_def);

void tcalc_variable_def_freev(void* var_def);
void tcalc_binary_func_def_freev(void* binary_func_def);
void tcalc_unary_func_def_freev(void* unary_func_def);
void tcalc_binary_op_def_freev(void* binary_op_def);
void tcalc_unary_op_def_freev(void* unary_op_def);

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

  if ((err = tcalc_ctx_add_unary_func(context, "sin", tcalc_sin)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "cos", tcalc_cos)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "tan", tcalc_tan)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "sec", tcalc_sec)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "csc", tcalc_csc)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "cot", tcalc_cot)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "asin", tcalc_asin)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "arcsin", tcalc_asin)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "acos", tcalc_acos)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "arccos", tcalc_acos)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "atan", tcalc_atan)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "arctan", tcalc_atan)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "asec", tcalc_asec)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "arcsec", tcalc_asec)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "acsc", tcalc_acsc)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "arccsc", tcalc_acsc)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "acot", tcalc_acot)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "arccot", tcalc_acot)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "sinh", tcalc_sinh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "cosh", tcalc_cosh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "tanh", tcalc_tanh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "asinh", tcalc_asinh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "arcsinh", tcalc_asinh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "acosh", tcalc_acosh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "arccosh", tcalc_acosh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "atanh", tcalc_atanh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "arctanh", tcalc_atanh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "log", tcalc_log)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "ln", tcalc_ln)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "exp", tcalc_exp)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "sqrt", tcalc_sqrt)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "cbrt", tcalc_cbrt)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "ceil", tcalc_ceil)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "floor", tcalc_floor)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "round", tcalc_round)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_func(context, "abs", tcalc_abs)) != TCALC_OK) goto cleanup;

  if ((err = tcalc_ctx_add_binary_func(context, "pow", tcalc_pow)) != TCALC_OK) goto cleanup;


  if ((err = tcalc_ctx_add_variable(context, "pi", M_PI)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_variable(context, "e", M_E)) != TCALC_OK) goto cleanup;

  if ((err = tcalc_ctx_add_unary_op(context, "+", 3, TCALC_RIGHT_ASSOCIATIVE, tcalc_unary_plus)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_unary_op(context, "-", 3, TCALC_RIGHT_ASSOCIATIVE, tcalc_unary_minus)) != TCALC_OK) goto cleanup;

  if ((err = tcalc_ctx_add_binary_op(context, "+", 1, TCALC_LEFT_ASSOCIATIVE, tcalc_add)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_binary_op(context, "-", 1, TCALC_LEFT_ASSOCIATIVE, tcalc_subtract)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_binary_op(context, "*", 2, TCALC_LEFT_ASSOCIATIVE, tcalc_multiply)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_binary_op(context, "/", 2, TCALC_LEFT_ASSOCIATIVE, tcalc_divide)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_binary_op(context, "%", 2, TCALC_LEFT_ASSOCIATIVE, tcalc_mod)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_ctx_add_binary_op(context, "^", 3, TCALC_RIGHT_ASSOCIATIVE, tcalc_pow)) != TCALC_OK) goto cleanup;
  
  *out = context;
  return err;

  cleanup:
    tcalc_ctx_free(context);
    return err;
}

void tcalc_ctx_free(tcalc_ctx* context) {
  TCALC_VEC_FREE_F(context->binary_funcs, tcalc_binary_func_def_free);
  TCALC_VEC_FREE_F(context->unary_funcs, tcalc_unary_func_def_free);
  TCALC_VEC_FREE_F(context->variables, tcalc_variable_def_free);
  TCALC_VEC_FREE_F(context->unary_ops, tcalc_unary_op_def_free);
  TCALC_VEC_FREE_F(context->binary_ops, tcalc_binary_op_def_free);

  free(context);
}

tcalc_err tcalc_ctx_add_variable(tcalc_ctx* context, char* name, double value) {
  for (size_t i = 0; i < context->variables.len; i++) {
    if (strcmp(context->variables.arr[i]->identifier, name) == 0) {
      context->variables.arr[i]->value = value;
      return TCALC_OK;
    }
  }

  tcalc_vardef* variable;
  tcalc_err err = tcalc_variable_def_alloc(name, value, &variable);
  if (err) return err;

  TCALC_VEC_PUSH(context->variables, variable, err);
  if (err) goto cleanup;

  return TCALC_OK;
  cleanup:
    tcalc_variable_def_free(variable);
    return err;
}

tcalc_err tcalc_ctx_add_unary_func(tcalc_ctx* context, char* name, tcalc_unfunc function) {
  for (size_t i = 0; i < context->unary_funcs.len; i++) {
    if (strcmp(context->unary_funcs.arr[i]->identifier, name) == 0) {
      context->unary_funcs.arr[i]->function = function;
      return TCALC_OK;
    }
  }

  tcalc_unfuncdef* unary_func;
  tcalc_err err = tcalc_unary_func_def_alloc(name, function, &unary_func);
  if (err) return err;

  TCALC_VEC_PUSH(context->unary_funcs, unary_func, err);
  if (err) goto cleanup;

  return TCALC_OK;
  cleanup:
    tcalc_unary_func_def_free(unary_func);
    return err;
}

tcalc_err tcalc_ctx_add_binary_func(tcalc_ctx* context, char* name, tcalc_binfunc function) {
  tcalc_binfuncdef* binary_func;
  tcalc_err err = tcalc_binary_func_def_alloc(name, function, &binary_func);
  if (err) return err;

  TCALC_VEC_PUSH(context->binary_funcs, binary_func, err);
  if (err) goto cleanup;

  return TCALC_OK;
  cleanup:
    tcalc_binary_func_def_free(binary_func);
    return err;
}

tcalc_err tcalc_ctx_add_unary_op(tcalc_ctx* context, char* name, int precedence, tcalc_assoc associativity, tcalc_unfunc function) {
  tcalc_uopdef* unary_op;
  tcalc_err err = tcalc_unary_op_def_alloc(name, precedence, associativity, function, &unary_op);
  if (err) return err;

  TCALC_VEC_PUSH(context->unary_ops, unary_op, err);
  if (err) goto cleanup;

  return TCALC_OK;
  cleanup:
    tcalc_unary_op_def_free(unary_op);
    return err;
}

tcalc_err tcalc_ctx_add_binary_op(tcalc_ctx* context,  char* name, int precedence, tcalc_assoc associativity, tcalc_binfunc function) {
  tcalc_binopdef* binary_op;
  tcalc_err err = tcalc_binary_op_def_alloc(name, precedence, associativity, function, &binary_op);
  if (err) return err;
  TCALC_VEC_PUSH(context->binary_ops, binary_op, err);
  if (err) goto cleanup;

  return TCALC_OK;
  cleanup:
    tcalc_binary_op_def_free(binary_op);
    return err;
}

int tcalc_ctx_has_identifier(const tcalc_ctx* context, const char* name) {
  return tcalc_ctx_has_variable(context, name) ||
  tcalc_ctx_has_func(context, name);
}

int tcalc_ctx_has_func(const tcalc_ctx* context, const char* name) {
  return tcalc_ctx_has_unary_func(context, name) ||
  tcalc_ctx_has_binary_func(context, name);
}

int tcalc_ctx_has_unary_func(const tcalc_ctx* context, const char* name) {
  return tcalc_ctx_get_unary_func(context, name, NULL) == TCALC_OK;
}

int tcalc_ctx_has_binary_func(const tcalc_ctx* context, const char* name) {
  return tcalc_ctx_get_binary_func(context, name, NULL) == TCALC_OK;
}

int tcalc_ctx_has_variable(const tcalc_ctx* context, const char* name) {
  return tcalc_ctx_get_variable(context, name, NULL) == TCALC_OK;
}

int tcalc_ctx_has_unary_op(const tcalc_ctx* context, const char* name) {
  return tcalc_ctx_get_unary_op(context, name, NULL) == TCALC_OK;
}

int tcalc_ctx_has_binary_op(const tcalc_ctx* context, const char* name) {
  return tcalc_ctx_get_binary_op(context, name, NULL) == TCALC_OK;
}

int tcalc_ctx_has_op(const tcalc_ctx* context, const char* name) {
  return tcalc_ctx_has_binary_op(context, name) || tcalc_ctx_has_unary_op(context, name);
}

tcalc_err tcalc_ctx_get_unary_func(const tcalc_ctx* context, const char* name, tcalc_unfuncdef** out) {
  for (size_t i = 0; i < context->unary_funcs.len; i++) {
    if (strcmp(context->unary_funcs.arr[i]->identifier, name) == 0) {
      if (out != NULL) *out = context->unary_funcs.arr[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

tcalc_err tcalc_ctx_get_binary_func(const tcalc_ctx* context, const char* name, tcalc_binfuncdef** out) {
  for (size_t i = 0; i < context->binary_funcs.len; i++) {
    if (strcmp(context->binary_funcs.arr[i]->identifier, name) == 0) {
      if (out != NULL) *out = context->binary_funcs.arr[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

tcalc_err tcalc_ctx_get_variable(const tcalc_ctx* context, const char* name, tcalc_vardef** out) {
  for (size_t i = 0; i < context->variables.len; i++) {
    if (strcmp(context->variables.arr[i]->identifier, name) == 0) {
      if (out != NULL) *out = context->variables.arr[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

tcalc_err tcalc_ctx_get_unary_op(const tcalc_ctx* context, const char* name, tcalc_uopdef** out) {
  for (size_t i = 0; i < context->unary_ops.len; i++) {
    if (strcmp(context->unary_ops.arr[i]->identifier, name) == 0) {
      if (out != NULL) *out = context->unary_ops.arr[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

tcalc_err tcalc_ctx_get_binary_op(const tcalc_ctx* context, const char* name, tcalc_binopdef** out) {
  for (size_t i = 0; i < context->binary_ops.len; i++) {
    if (strcmp(context->binary_ops.arr[i]->identifier, name) == 0) {
      if (out != NULL) *out = context->binary_ops.arr[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

tcalc_err tcalc_variable_def_alloc(char* name, double value, tcalc_vardef** out) {
  tcalc_vardef* var_def = (tcalc_vardef*)malloc(sizeof(tcalc_vardef));
  if (var_def == NULL) return TCALC_BAD_ALLOC;

  tcalc_err err = tcalc_strdup(name, &var_def->identifier);
  if (err) {
    free(var_def);
    return err;
  }

  var_def->value = value;
  *out = var_def;
  return TCALC_OK;
}

tcalc_err tcalc_binary_func_def_alloc(char* name, tcalc_binfunc function, tcalc_binfuncdef** out) {
  tcalc_binfuncdef* binary_func = (tcalc_binfuncdef*)malloc(sizeof(tcalc_binfuncdef));
  if (binary_func == NULL) return TCALC_BAD_ALLOC;

  tcalc_err err = tcalc_strdup(name, &binary_func->identifier);
  if (err) {
    free(binary_func);
    return err;
  }

  binary_func->function = function;
  *out = binary_func;
  return TCALC_OK;
}

tcalc_err tcalc_unary_func_def_alloc(char* name, tcalc_unfunc function, tcalc_unfuncdef** out) {
  tcalc_unfuncdef* unary_func = (tcalc_unfuncdef*)malloc(sizeof(tcalc_unfuncdef));
  if (unary_func == NULL) return TCALC_BAD_ALLOC;

  tcalc_err err = tcalc_strdup(name, &unary_func->identifier);
  if (err) {
    free(unary_func);
    return err;
  }

  unary_func->function = function;
  *out = unary_func;
  return TCALC_OK;
}

tcalc_err tcalc_binary_op_def_alloc(char* name, int precedence, tcalc_assoc associativity, tcalc_binfunc function, tcalc_binopdef** out) {
  tcalc_binopdef* binary_op_def = (tcalc_binopdef*)malloc(sizeof(tcalc_binopdef));
  if (binary_op_def == NULL) return TCALC_BAD_ALLOC;

  tcalc_err err = tcalc_strdup(name, &binary_op_def->identifier);
  if (err) {
    free(binary_op_def);
    return err;
  }

  binary_op_def->precedence = precedence;
  binary_op_def->associativity = associativity;
  binary_op_def->function = function;
  *out = binary_op_def;
  return TCALC_OK;
}

tcalc_err tcalc_unary_op_def_alloc(char* name, int precedence, tcalc_assoc associativity, tcalc_unfunc function, tcalc_uopdef** out) {
  tcalc_uopdef* unary_op_def = (tcalc_uopdef*)malloc(sizeof(tcalc_uopdef));
  if (unary_op_def == NULL) return TCALC_BAD_ALLOC;

  tcalc_err err = tcalc_strdup(name, &unary_op_def->identifier);
  if (err) {
    free(unary_op_def);
    return err;
  }

  unary_op_def->precedence = precedence;
  unary_op_def->associativity = associativity;
  unary_op_def->function = function;
  *out = unary_op_def;
  return TCALC_OK;
}

void tcalc_variable_def_free(tcalc_vardef* var_def) {
  if (var_def == NULL) return;
  free(var_def->identifier);
  free(var_def);
}

void tcalc_binary_func_def_free(tcalc_binfuncdef* binary_func_def) {
  if (binary_func_def == NULL) return;
  free(binary_func_def->identifier);
  free(binary_func_def);
}

void tcalc_unary_func_def_free(tcalc_unfuncdef* unary_func_def) {
  if (unary_func_def == NULL) return;
  free(unary_func_def->identifier);
  free(unary_func_def);
}

void tcalc_binary_op_def_free(tcalc_binopdef* binary_op_def) {
  if (binary_op_def == NULL) return;
  free(binary_op_def->identifier);
  free(binary_op_def);
}

void tcalc_unary_op_def_free(tcalc_uopdef* unary_op_def) {
  if (unary_op_def == NULL) return;
  free(unary_op_def->identifier);
  free(unary_op_def);
}

void tcalc_variable_def_freev(void* var_def) {
  tcalc_variable_def_free((tcalc_vardef*)var_def);
}

void tcalc_binary_func_def_freev(void* binary_func_def) {
  tcalc_binary_func_def_free((tcalc_binfuncdef*)binary_func_def);
}

void tcalc_unary_func_def_freev(void* unary_func_def) {
  tcalc_unary_func_def_free((tcalc_unfuncdef*)unary_func_def);
}

void tcalc_binary_op_def_freev(void* binary_op_def) {
  tcalc_binary_op_def_free((tcalc_binopdef*)binary_op_def);
}

void tcalc_unary_op_def_freev(void* unary_op_def) {
  tcalc_unary_op_def_free((tcalc_uopdef*)unary_op_def);
}

tcalc_opdata tcalc_binary_op_get_data(tcalc_binopdef* binary_op_def) {
  tcalc_opdata op_data;
  op_data.precedence = binary_op_def->precedence;
  op_data.associativity = binary_op_def->associativity;
  return op_data;
}

tcalc_opdata tcalc_unary_op_get_data(tcalc_uopdef* unary_op_def) {
  tcalc_opdata op_data;
  op_data.precedence = unary_op_def->precedence;
  op_data.associativity = unary_op_def->associativity;
  return op_data;
}