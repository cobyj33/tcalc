#include "tcalc_context.h"

#include "tcalc_constants.h"
#include "tcalc_func.h"
#include "tcalc_mem.h"
#include "tcalc_string.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

tcalc_error_t tcalc_variable_def_alloc(char* name, double value, tcalc_variable_def_t** out);
tcalc_error_t tcalc_binary_func_def_alloc(char* name, tcalc_binary_func function, tcalc_binary_func_def_t** out);
tcalc_error_t tcalc_unary_func_def_alloc(char* name, tcalc_unary_func function, tcalc_unary_func_def_t** out);

void tcalc_variable_def_free(tcalc_variable_def_t* var_def);
void tcalc_binary_func_def_free(tcalc_binary_func_def_t* binary_func_def);
void tcalc_unary_func_def_free(tcalc_unary_func_def_t* unary_func_def);

void tcalc_variable_def_freev(void* var_def);
void tcalc_binary_func_def_freev(void* binary_func_def);
void tcalc_unary_func_def_freev(void* unary_func_def);

tcalc_error_t tcalc_context_alloc_empty(tcalc_context_t** out) {
  tcalc_context_t* context = (tcalc_context_t*)calloc(1, sizeof(tcalc_context_t));
  if (context == NULL) return TCALC_BAD_ALLOC;

  *out = context;
  return TCALC_OK; 
}

tcalc_error_t tcalc_context_alloc_default(tcalc_context_t** out) {
  tcalc_context_t* context;
  tcalc_error_t err = tcalc_context_alloc_empty(&context);
  if (err) return err;

  if ((err = tcalc_context_add_unary_func(context, "sin", tcalc_sin)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "cos", tcalc_cos)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "tan", tcalc_tan)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "sec", tcalc_sec)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "csc", tcalc_csc)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "cot", tcalc_cot)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "asin", tcalc_asin)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "arcsin", tcalc_asin)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "acos", tcalc_acos)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "arccos", tcalc_acos)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "atan", tcalc_atan)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "arctan", tcalc_atan)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "asec", tcalc_asec)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "arcsec", tcalc_asec)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "acsc", tcalc_acsc)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "arccsc", tcalc_acsc)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "acot", tcalc_acot)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "arccot", tcalc_acot)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "sinh", tcalc_sinh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "cosh", tcalc_cosh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "tanh", tcalc_tanh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "asinh", tcalc_asinh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "arcsinh", tcalc_asinh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "acosh", tcalc_acosh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "arccosh", tcalc_acosh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "atanh", tcalc_atanh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "arctanh", tcalc_atanh)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "log", tcalc_log)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "ln", tcalc_ln)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "exp", tcalc_exp)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "sqrt", tcalc_sqrt)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "cbrt", tcalc_cbrt)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "ceil", tcalc_ceil)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "floor", tcalc_floor)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "round", tcalc_round)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_unary_func(context, "abs", tcalc_abs)) != TCALC_OK) goto cleanup;

  if ((err = tcalc_context_add_binary_func(context, "pow", tcalc_pow)) != TCALC_OK) goto cleanup;

  if ((err = tcalc_context_add_variable(context, "pi", M_PI)) != TCALC_OK) goto cleanup;
  if ((err = tcalc_context_add_variable(context, "e", M_E)) != TCALC_OK) goto cleanup;

  *out = context;
  return err;

  cleanup:
    tcalc_context_free(context);
    return err;
}

void tcalc_context_free(tcalc_context_t* context) {
  if (context == NULL) return;
  tcalc_free_arr((void**)context->binary_funcs, context->nb_binary_funcs, tcalc_binary_func_def_freev);
  tcalc_free_arr((void**)context->unary_funcs, context->nb_unary_funcs, tcalc_unary_func_def_freev);
  tcalc_free_arr((void**)context->variables, context->nb_variables, tcalc_variable_def_freev);
  free(context);
}

tcalc_error_t tcalc_context_add_variable(tcalc_context_t* context, char* name, double value) {
  tcalc_variable_def_t* variable;
  tcalc_error_t err = tcalc_variable_def_alloc(name, value, &variable);
  if (err) return err;

  TCALC_DARR_PUSH(context->variables, context->nb_variables, context->variables_capacity, variable, err)
  if (err) goto cleanup;

  // if ((err = tcalc_alloc_grow((void**)&context->variables, sizeof(tcalc_variable_def_t*), context->nb_variables + 1, &context->variables_capacity)) != TCALC_OK)
  //   goto cleanup;
  // context->variables[context->nb_variables++] = variable;
  
  return err;

  cleanup:
    tcalc_variable_def_free(variable);
    return err;
}

tcalc_error_t tcalc_context_add_unary_func(tcalc_context_t* context, char* name, tcalc_unary_func function) {
  tcalc_unary_func_def_t* unary_func;
  tcalc_error_t err = tcalc_unary_func_def_alloc(name, function, &unary_func);
  if (err) return err;

  TCALC_DARR_PUSH(context->unary_funcs, context->nb_unary_funcs, context->unary_funcs_capacity, unary_func, err)
  if (err) goto cleanup;

  return err;

  cleanup:
    tcalc_unary_func_def_free(unary_func);
    return err;
}

tcalc_error_t tcalc_context_add_binary_func(tcalc_context_t* context, char* name, tcalc_binary_func function) {
  tcalc_binary_func_def_t* binary_func;
  tcalc_error_t err = tcalc_binary_func_def_alloc(name, function, &binary_func);
  if (err) return err;

  TCALC_DARR_PUSH(context->binary_funcs, context->nb_binary_funcs, context->binary_funcs_capacity, binary_func, err)
  if (err) goto cleanup;

  return err;

  cleanup:
    tcalc_binary_func_def_free(binary_func);
    return err;
}

int tcalc_context_has_identifier(const tcalc_context_t* context, const char* name) {
  return tcalc_context_has_variable(context, name) ||
  tcalc_context_has_func(context, name);
}

int tcalc_context_has_func(const tcalc_context_t* context, const char* name) {
  return tcalc_context_has_unary_func(context, name) ||
  tcalc_context_has_binary_func(context, name);
}

int tcalc_context_has_unary_func(const tcalc_context_t* context, const char* name) {
  return tcalc_context_get_unary_func(context, name, NULL) == TCALC_OK;
}

int tcalc_context_has_binary_func(const tcalc_context_t* context, const char* name) {
  return tcalc_context_get_binary_func(context, name, NULL) == TCALC_OK;
}

int tcalc_context_has_variable(const tcalc_context_t* context, const char* name) {
  return tcalc_context_get_variable(context, name, NULL) == TCALC_OK;
}

tcalc_error_t tcalc_context_get_unary_func(const tcalc_context_t* context, const char* name, tcalc_unary_func_def_t** out) {
  for (size_t i = 0; i < context->nb_unary_funcs; i++) {
    if (strcmp(context->unary_funcs[i]->identifier, name) == 0) {
      if (out != NULL) *out = context->unary_funcs[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

tcalc_error_t tcalc_context_get_binary_func(const tcalc_context_t* context, const char* name, tcalc_binary_func_def_t** out) {
  for (size_t i = 0; i < context->nb_binary_funcs; i++) {
    if (strcmp(context->binary_funcs[i]->identifier, name) == 0) {
      if (out != NULL) *out = context->binary_funcs[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

tcalc_error_t tcalc_context_get_variable(const tcalc_context_t* context, const char* name, tcalc_variable_def_t** out) {
  for (size_t i = 0; i < context->nb_variables; i++) {
    if (strcmp(context->variables[i]->identifier, name) == 0) {
      if (out != NULL) *out = context->variables[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

tcalc_error_t tcalc_variable_def_alloc(char* name, double value, tcalc_variable_def_t** out) {
  tcalc_variable_def_t* var_def = (tcalc_variable_def_t*)malloc(sizeof(tcalc_variable_def_t));
  if (var_def == NULL) return TCALC_BAD_ALLOC;

  tcalc_error_t err = tcalc_strdup(name, &var_def->identifier);
  if (err) {
    free(var_def);
    return TCALC_BAD_ALLOC;
  }

  var_def->value = value;
  *out = var_def;
  return TCALC_OK;
}

tcalc_error_t tcalc_binary_func_def_alloc(char* name, tcalc_binary_func function, tcalc_binary_func_def_t** out) {
  tcalc_binary_func_def_t* binary_func = (tcalc_binary_func_def_t*)malloc(sizeof(tcalc_binary_func_def_t));
  if (binary_func == NULL) return TCALC_BAD_ALLOC;

  tcalc_error_t err = tcalc_strdup(name, &binary_func->identifier);
  if (err) {
    free(binary_func);
    return TCALC_BAD_ALLOC;
  }

  binary_func->function = function;
  *out = binary_func;
  return TCALC_OK;
}

tcalc_error_t tcalc_unary_func_def_alloc(char* name, tcalc_unary_func function, tcalc_unary_func_def_t** out) {
  tcalc_unary_func_def_t* unary_func = (tcalc_unary_func_def_t*)malloc(sizeof(tcalc_unary_func_def_t));
  if (unary_func == NULL) return TCALC_BAD_ALLOC;

  tcalc_error_t err = tcalc_strdup(name, &unary_func->identifier);
  if (err) {
    free(unary_func);
    return TCALC_BAD_ALLOC;
  }

  unary_func->function = function;
  *out = unary_func;
  return TCALC_OK;
}

void tcalc_variable_def_free(tcalc_variable_def_t* var_def) {
  if (var_def == NULL) return;
  free(var_def->identifier);
  free(var_def);
}

void tcalc_binary_func_def_free(tcalc_binary_func_def_t* binary_func_def) {
  if (binary_func_def == NULL) return;
  free(binary_func_def->identifier);
  free(binary_func_def);
}

void tcalc_unary_func_def_free(tcalc_unary_func_def_t* unary_func_def) {
  if (unary_func_def == NULL) return;
  free(unary_func_def->identifier);
  free(unary_func_def);
}

void tcalc_variable_def_freev(void* var_def) {
  tcalc_variable_def_free((tcalc_variable_def_t*)var_def);
}

void tcalc_binary_func_def_freev(void* binary_func_def) {
  tcalc_binary_func_def_free((tcalc_binary_func_def_t*)binary_func_def);
}

void tcalc_unary_func_def_freev(void* unary_func_def) {
  tcalc_unary_func_def_free((tcalc_unary_func_def_t*)unary_func_def);
}
