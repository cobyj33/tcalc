#include "tcalc_context.h"
#include "tcalc_constants.h"
#include "tcalc_func.h"

const tcalc_context_t tcalc_global_context = {
  {
    {'+', {4, TCALC_RIGHT_ASSOCIATIVE}, tcalc_unary_plus},
    {'-', {4, TCALC_RIGHT_ASSOCIATIVE}, tcalc_unary_minus},
  },
  {
    {'+', {1, TCALC_LEFT_ASSOCIATIVE}, tcalc_add},
    {'-', {1, TCALC_LEFT_ASSOCIATIVE}, tcalc_subtract},
    {'*', {2, TCALC_LEFT_ASSOCIATIVE}, tcalc_multiply},
    {'/', {2, TCALC_LEFT_ASSOCIATIVE}, tcalc_divide},
    {'%', {2, TCALC_LEFT_ASSOCIATIVE}, tcalc_mod},
    {'^', {3, TCALC_RIGHT_ASSOCIATIVE}, tcalc_pow},
  },
  {
    {'(', ')'}
  },
  {
    {"sin", tcalc_sin},
    {"cos", tcalc_cos},
    {"tan", tcalc_tan},
    {"asin", tcalc_asin},
    {"arcsin", tcalc_asin},
    {"acos", tcalc_acos},
    {"arccos", tcalc_acos},
    {"atan", tcalc_atan},
    {"arctan", tcalc_atan},
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
  },
  {
    {"pow", tcalc_pow},
  },
  {
    {"pi", M_PI},
    {"e", M_E}
  },
};