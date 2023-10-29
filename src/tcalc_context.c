#include "tcalc_context.h"

#include "tcalc_constants.h"
#include "tcalc_func.h"

#include <stddef.h>
#include <string.h>


const tcalc_context_t TCALC_GLOBAL_CONTEXT = {
  {
    {"sin", tcalc_sin},
    {"cos", tcalc_cos},
    {"tan", tcalc_tan},
    {"sec", tcalc_sec},
    {"csc", tcalc_csc},
    {"cot", tcalc_cot},
    {"asin", tcalc_asin},
    {"arcsin", tcalc_asin},
    {"acos", tcalc_acos},
    {"arccos", tcalc_acos},
    {"atan", tcalc_atan},
    {"arctan", tcalc_atan},
    {"asec", tcalc_asec},
    {"arcsec", tcalc_asec},
    {"acsc", tcalc_acsc},
    {"arccsc", tcalc_acsc},
    {"acot", tcalc_acot},
    {"arccot", tcalc_acot},
    {"sinh", tcalc_sinh},
    {"cosh", tcalc_cosh},
    {"tanh", tcalc_tanh},
    {"asinh", tcalc_asinh},
    {"arcsinh", tcalc_asinh},
    {"acosh", tcalc_acosh},
    {"arccosh", tcalc_acosh},
    {"atanh", tcalc_atanh},
    {"arctanh", tcalc_atanh},
    {"log", tcalc_log},
    {"ln", tcalc_ln},
    {"exp", tcalc_exp},
    {"sqrt", tcalc_sqrt},
    {"cbrt", tcalc_cbrt},
    {"ceil", tcalc_ceil},
    {"floor", tcalc_floor},
    {"round", tcalc_round},
    {"abs", tcalc_abs},
  }, 36,
  {
    {"pow", tcalc_pow},
  }, 1,
  {
    {"pi", M_PI},
    {"e", M_E}
  }, 2
};

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

tcalc_error_t tcalc_context_get_unary_func(const tcalc_context_t* context, const char* name, tcalc_unary_func_def_t* out) {
  for (size_t i = 0; i < context->nb_unary_funcs; i++) {
    if (strcmp(context->unary_funcs[i].identifier, name) == 0) {
      if (out != NULL) *out = context->unary_funcs[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

tcalc_error_t tcalc_context_get_binary_func(const tcalc_context_t* context, const char* name, tcalc_binary_func_def_t* out) {
  for (size_t i = 0; i < context->nb_binary_funcs; i++) {
    if (strcmp(context->binary_funcs[i].identifier, name) == 0) {
      if (out != NULL) *out = context->binary_funcs[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

tcalc_error_t tcalc_context_get_variable(const tcalc_context_t* context, const char* name, tcalc_variable_def_t* out) {
  for (size_t i = 0; i < context->nb_variables; i++) {
    if (strcmp(context->variables[i].identifier, name) == 0) {
      if (out != NULL) *out = context->variables[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}