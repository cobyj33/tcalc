#include "tcalc.h"

#include <stdbool.h>
#include <math.h>
#include <errno.h>
#include <float.h>
#include <assert.h>

// I pray that no debugger ever has to grace this macro abomination for any
// reason and everything just always works well.

#define tcalc_val_unlopfunc_impl(_funcname_, _implfuncname_) \
  tcalc_err _funcname_(tcalc_val a, bool* out) { \
    assert(out != NULL); \
    if (a.type != TCALC_VALTYPE_BOOL) \
      return TCALC_ERR_BAD_CAST; \
    \
    *out = _implfuncname_(a.as.boolean); \
    return TCALC_ERR_OK; \
  }

tcalc_val_unlopfunc_impl(tcalc_val_not, tcalc_not)

#define tcalc_val_binlopfunc_impl(_funcname_, _implfuncname_) \
  tcalc_err _funcname_(tcalc_val a, tcalc_val b, bool* out) { \
    assert(out != NULL); \
    if (a.type != TCALC_VALTYPE_BOOL || b.type != TCALC_VALTYPE_BOOL) \
      return TCALC_ERR_BAD_CAST; \
    \
    *out = _implfuncname_(a.as.boolean, b.as.boolean); \
    return TCALC_ERR_OK; \
  }

tcalc_val_binlopfunc_impl(tcalc_val_and, tcalc_and)
tcalc_val_binlopfunc_impl(tcalc_val_or, tcalc_or)
tcalc_val_binlopfunc_impl(tcalc_val_nand, tcalc_nand)
tcalc_val_binlopfunc_impl(tcalc_val_nor, tcalc_nor)
tcalc_val_binlopfunc_impl(tcalc_val_xor, tcalc_xor)
tcalc_val_binlopfunc_impl(tcalc_val_xnor, tcalc_xnor)
tcalc_val_binlopfunc_impl(tcalc_val_matcond, tcalc_matcond)
tcalc_val_binlopfunc_impl(tcalc_val_equals_l, tcalc_equals_l)
tcalc_val_binlopfunc_impl(tcalc_val_nequals_l, tcalc_nequals_l)

#define tcalc_val_relopfunc_impl(_funcname_, _implfuncname_) \
  tcalc_err _funcname_(tcalc_val a, tcalc_val b, bool* out) { \
    assert(out != NULL); \
    if (a.type != TCALC_VALTYPE_NUM || b.type != TCALC_VALTYPE_NUM) \
      return TCALC_ERR_BAD_CAST; \
    \
    *out = _implfuncname_(a.as.num, b.as.num); \
    return TCALC_ERR_OK; \
  }

// Relational Functions
tcalc_val_relopfunc_impl(tcalc_val_equals, tcalc_equals)
tcalc_val_relopfunc_impl(tcalc_val_nequals, tcalc_nequals)
tcalc_val_relopfunc_impl(tcalc_val_lt, tcalc_lt)
tcalc_val_relopfunc_impl(tcalc_val_lteq, tcalc_lteq)
tcalc_val_relopfunc_impl(tcalc_val_gt, tcalc_gt)
tcalc_val_relopfunc_impl(tcalc_val_gteq, tcalc_gteq)

#define tcalc_val_unopfunc_impl(_funcname_, _implfuncname_) \
  tcalc_err _funcname_(tcalc_val a, double* out) { \
    assert(out != NULL); \
    if (a.type != TCALC_VALTYPE_NUM) \
      return TCALC_ERR_BAD_CAST; \
    \
    return _implfuncname_(a.as.num, out); \
  }

tcalc_val_unopfunc_impl(tcalc_val_ceil, tcalc_ceil)
tcalc_val_unopfunc_impl(tcalc_val_floor, tcalc_floor)
tcalc_val_unopfunc_impl(tcalc_val_round, tcalc_round)
tcalc_val_unopfunc_impl(tcalc_val_abs, tcalc_abs)

tcalc_val_unopfunc_impl(tcalc_val_sin, tcalc_sin)
tcalc_val_unopfunc_impl(tcalc_val_cos, tcalc_cos)
tcalc_val_unopfunc_impl(tcalc_val_tan, tcalc_tan)
tcalc_val_unopfunc_impl(tcalc_val_sec, tcalc_sec)
tcalc_val_unopfunc_impl(tcalc_val_csc, tcalc_csc)
tcalc_val_unopfunc_impl(tcalc_val_cot, tcalc_cot)
tcalc_val_unopfunc_impl(tcalc_val_asin, tcalc_asin)
tcalc_val_unopfunc_impl(tcalc_val_acos, tcalc_acos)
tcalc_val_unopfunc_impl(tcalc_val_atan, tcalc_atan)
tcalc_val_unopfunc_impl(tcalc_val_asec, tcalc_asec)
tcalc_val_unopfunc_impl(tcalc_val_acsc, tcalc_acsc)
tcalc_val_unopfunc_impl(tcalc_val_acot, tcalc_acot)
tcalc_val_unopfunc_impl(tcalc_val_sinh, tcalc_sinh)
tcalc_val_unopfunc_impl(tcalc_val_cosh, tcalc_cosh)
tcalc_val_unopfunc_impl(tcalc_val_tanh, tcalc_tanh)
tcalc_val_unopfunc_impl(tcalc_val_asinh, tcalc_asinh)
tcalc_val_unopfunc_impl(tcalc_val_acosh, tcalc_acosh)
tcalc_val_unopfunc_impl(tcalc_val_atanh, tcalc_atanh)

tcalc_val_unopfunc_impl(tcalc_val_sin_deg, tcalc_sin_deg)
tcalc_val_unopfunc_impl(tcalc_val_cos_deg, tcalc_cos_deg)
tcalc_val_unopfunc_impl(tcalc_val_tan_deg, tcalc_tan_deg)
tcalc_val_unopfunc_impl(tcalc_val_sec_deg, tcalc_sec_deg)
tcalc_val_unopfunc_impl(tcalc_val_csc_deg, tcalc_csc_deg)
tcalc_val_unopfunc_impl(tcalc_val_cot_deg, tcalc_cot_deg)
tcalc_val_unopfunc_impl(tcalc_val_asin_deg, tcalc_asin_deg)
tcalc_val_unopfunc_impl(tcalc_val_acos_deg, tcalc_acos_deg)
tcalc_val_unopfunc_impl(tcalc_val_atan_deg, tcalc_atan_deg)
tcalc_val_unopfunc_impl(tcalc_val_asec_deg, tcalc_asec_deg)
tcalc_val_unopfunc_impl(tcalc_val_acsc_deg, tcalc_acsc_deg)
tcalc_val_unopfunc_impl(tcalc_val_acot_deg, tcalc_acot_deg)
tcalc_val_unopfunc_impl(tcalc_val_sinh_deg, tcalc_sinh_deg)
tcalc_val_unopfunc_impl(tcalc_val_cosh_deg, tcalc_cosh_deg)
tcalc_val_unopfunc_impl(tcalc_val_tanh_deg, tcalc_tanh_deg)
tcalc_val_unopfunc_impl(tcalc_val_asinh_deg, tcalc_asinh_deg)
tcalc_val_unopfunc_impl(tcalc_val_acosh_deg, tcalc_acosh_deg)
tcalc_val_unopfunc_impl(tcalc_val_atanh_deg, tcalc_atanh_deg)

tcalc_val_unopfunc_impl(tcalc_val_unary_plus, tcalc_unary_plus)
tcalc_val_unopfunc_impl(tcalc_val_unary_minus, tcalc_unary_minus)

tcalc_val_unopfunc_impl(tcalc_val_log, tcalc_log)
tcalc_val_unopfunc_impl(tcalc_val_sqrt, tcalc_sqrt)
tcalc_val_unopfunc_impl(tcalc_val_cbrt, tcalc_cbrt)
tcalc_val_unopfunc_impl(tcalc_val_ln, tcalc_ln)
tcalc_val_unopfunc_impl(tcalc_val_exp, tcalc_exp)

#define tcalc_val_binopfunc_impl(_funcname_, _implfuncname_) \
  tcalc_err _funcname_(tcalc_val a, tcalc_val b, double* out) { \
    assert(out != NULL); \
    if (a.type != TCALC_VALTYPE_NUM || b.type != TCALC_VALTYPE_NUM) \
      return TCALC_ERR_BAD_CAST; \
    \
    return _implfuncname_(a.as.num, b.as.num, out); \
  }

tcalc_val_binopfunc_impl(tcalc_val_add, tcalc_add)
tcalc_val_binopfunc_impl(tcalc_val_subtract, tcalc_subtract)
tcalc_val_binopfunc_impl(tcalc_val_multiply, tcalc_multiply)
tcalc_val_binopfunc_impl(tcalc_val_divide, tcalc_divide)
tcalc_val_binopfunc_impl(tcalc_val_mod, tcalc_mod)

tcalc_val_binopfunc_impl(tcalc_val_pow, tcalc_pow)
tcalc_val_binopfunc_impl(tcalc_val_atan2, tcalc_atan2)
tcalc_val_binopfunc_impl(tcalc_val_atan2_deg, tcalc_atan2_deg)
