#include "tcalc.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("log"), tcalc_val_log));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("ln"), tcalc_val_ln));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("exp"), tcalc_val_exp));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("sqrt"), tcalc_val_sqrt));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("cbrt"), tcalc_val_cbrt));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("ceil"), tcalc_val_ceil));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("floor"), tcalc_val_floor));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("round"), tcalc_val_round));
  cleanup_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("abs"), tcalc_val_abs));

  // Default Binary Functions:
  cleanup_on_err(err, tcalc_ctx_addbinfunc(ctx, TCALC_STRLIT_PTR_LEN("pow"), tcalc_val_pow));

  // Default Variables:
  cleanup_on_err(err, tcalc_ctx_addvar(ctx, TCALC_STRLIT_PTR_LEN("pi"), TCALC_VAL_INIT_NUM(TCALC_PI)));
  cleanup_on_err(err, tcalc_ctx_addvar(ctx, TCALC_STRLIT_PTR_LEN("e"), TCALC_VAL_INIT_NUM(TCALC_E)));

  // Default Logical Variables:
  cleanup_on_err(err, tcalc_ctx_addvar(ctx, TCALC_STRLIT_PTR_LEN("true"), TCALC_VAL_INIT_BOOL(1)));
  cleanup_on_err(err, tcalc_ctx_addvar(ctx, TCALC_STRLIT_PTR_LEN("false"), TCALC_VAL_INIT_BOOL(0)));

  // Default Logical Unary Operators:
  cleanup_on_err(err, tcalc_ctx_addunlop(ctx, TCALC_STRLIT_PTR_LEN("!"), 10, TCALC_RIGHT_ASSOC, tcalc_val_not));

  // Default Unary Operators:
  cleanup_on_err(err, tcalc_ctx_addunop(ctx, TCALC_STRLIT_PTR_LEN("+"), 10, TCALC_RIGHT_ASSOC, tcalc_val_unary_plus));
  cleanup_on_err(err, tcalc_ctx_addunop(ctx, TCALC_STRLIT_PTR_LEN("-"), 10, TCALC_RIGHT_ASSOC, tcalc_val_unary_minus));

  // Default Binary Operators:
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, TCALC_STRLIT_PTR_LEN("**"), 10, TCALC_RIGHT_ASSOC, tcalc_val_pow));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, TCALC_STRLIT_PTR_LEN("^"), 10, TCALC_RIGHT_ASSOC, tcalc_val_pow));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, TCALC_STRLIT_PTR_LEN(""), 9, TCALC_LEFT_ASSOC, tcalc_val_multiply)); // implicit multiplication (0-length binary operation)
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, TCALC_STRLIT_PTR_LEN("*"), 9, TCALC_LEFT_ASSOC, tcalc_val_multiply));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, TCALC_STRLIT_PTR_LEN("/"), 9, TCALC_LEFT_ASSOC, tcalc_val_divide));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, TCALC_STRLIT_PTR_LEN("%"), 9, TCALC_LEFT_ASSOC, tcalc_val_mod));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, TCALC_STRLIT_PTR_LEN("+"), 8, TCALC_LEFT_ASSOC, tcalc_val_add));
  cleanup_on_err(err, tcalc_ctx_addbinop(ctx, TCALC_STRLIT_PTR_LEN("-"), 8, TCALC_LEFT_ASSOC, tcalc_val_subtract));

  // Default Relational Binary Operators:
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, TCALC_STRLIT_PTR_LEN("<"), 6, TCALC_LEFT_ASSOC, tcalc_val_lt));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, TCALC_STRLIT_PTR_LEN("<="), 6, TCALC_LEFT_ASSOC, tcalc_val_lteq));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, TCALC_STRLIT_PTR_LEN(">"), 6, TCALC_LEFT_ASSOC, tcalc_val_gt));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, TCALC_STRLIT_PTR_LEN(">="), 6, TCALC_LEFT_ASSOC, tcalc_val_gteq));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, TCALC_STRLIT_PTR_LEN("="), 5, TCALC_RIGHT_ASSOC, tcalc_val_equals));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, TCALC_STRLIT_PTR_LEN("=="), 5, TCALC_RIGHT_ASSOC, tcalc_val_equals));
  cleanup_on_err(err, tcalc_ctx_addrelop(ctx, TCALC_STRLIT_PTR_LEN("!="), 5, TCALC_RIGHT_ASSOC, tcalc_val_nequals));



  // Default Logical Binary Operators:
  cleanup_on_err(err, tcalc_ctx_addbinlop(ctx, TCALC_STRLIT_PTR_LEN("="), 5, TCALC_RIGHT_ASSOC, tcalc_val_equals_l));
  cleanup_on_err(err, tcalc_ctx_addbinlop(ctx, TCALC_STRLIT_PTR_LEN("=="), 5, TCALC_RIGHT_ASSOC, tcalc_val_equals_l));
  cleanup_on_err(err, tcalc_ctx_addbinlop(ctx, TCALC_STRLIT_PTR_LEN("!="), 5, TCALC_RIGHT_ASSOC, tcalc_val_nequals_l));
  cleanup_on_err(err, tcalc_ctx_addbinlop(ctx, TCALC_STRLIT_PTR_LEN("&&"), 4, TCALC_RIGHT_ASSOC, tcalc_val_and));
  cleanup_on_err(err, tcalc_ctx_addbinlop(ctx, TCALC_STRLIT_PTR_LEN("||"), 3, TCALC_RIGHT_ASSOC, tcalc_val_or));

  *out = ctx;
  return err;

  cleanup:
    tcalc_ctx_free(ctx);
    return err;
}

tcalc_err tcalc_ctx_addtrigrad(tcalc_ctx* ctx) {
  tcalc_err err = TCALC_ERR_OK;
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("sin"), tcalc_val_sin));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("cos"), tcalc_val_cos));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("tan"), tcalc_val_tan));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("sec"), tcalc_val_sec));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("csc"), tcalc_val_csc));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("cot"), tcalc_val_cot));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("asin"), tcalc_val_asin));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arcsin"), tcalc_val_asin));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("acos"), tcalc_val_acos));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arccos"), tcalc_val_acos));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("atan"), tcalc_val_atan));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arctan"), tcalc_val_atan));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("asec"), tcalc_val_asec));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arcsec"), tcalc_val_asec));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("acsc"), tcalc_val_acsc));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arccsc"), tcalc_val_acsc));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("acot"), tcalc_val_acot));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arccot"), tcalc_val_acot));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("sinh"), tcalc_val_sinh));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("cosh"), tcalc_val_cosh));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("tanh"), tcalc_val_tanh));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("asinh"), tcalc_val_asinh));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arcsinh"), tcalc_val_asinh));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("acosh"), tcalc_val_acosh));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arccosh"), tcalc_val_acosh));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("atanh"), tcalc_val_atanh));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arctanh"), tcalc_val_atanh));
  ret_on_err(err, tcalc_ctx_addbinfunc(ctx, TCALC_STRLIT_PTR_LEN("atan2"), tcalc_val_atan2));
  return err;
}

tcalc_err tcalc_ctx_addtrigdeg(tcalc_ctx* ctx) {
  tcalc_err err = TCALC_ERR_OK;
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("sin"), tcalc_val_sin_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("cos"), tcalc_val_cos_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("tan"), tcalc_val_tan_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("sec"), tcalc_val_sec_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("csc"), tcalc_val_csc_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("cot"), tcalc_val_cot_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("asin"), tcalc_val_asin_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arcsin"), tcalc_val_asin_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("acos"), tcalc_val_acos_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arccos"), tcalc_val_acos_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("atan"), tcalc_val_atan_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arctan"), tcalc_val_atan_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("asec"), tcalc_val_asec_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arcsec"), tcalc_val_asec_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("acsc"), tcalc_val_acsc_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arccsc"), tcalc_val_acsc_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("acot"), tcalc_val_acot_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arccot"), tcalc_val_acot_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("sinh"), tcalc_val_sinh_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("cosh"), tcalc_val_cosh_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("tanh"), tcalc_val_tanh_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("asinh"), tcalc_val_asinh_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arcsinh"), tcalc_val_asinh_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("acosh"), tcalc_val_acosh_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arccosh"), tcalc_val_acosh_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("atanh"), tcalc_val_atanh_deg));
  ret_on_err(err, tcalc_ctx_addunfunc(ctx, TCALC_STRLIT_PTR_LEN("arctanh"), tcalc_val_atanh_deg));
  ret_on_err(err, tcalc_ctx_addbinfunc(ctx, TCALC_STRLIT_PTR_LEN("atan2"), tcalc_val_atan2_deg));
  return err;
}

void tcalc_ctx_free(tcalc_ctx* ctx) {
  if (ctx == NULL) return;
  TCALC_VEC_FREE(ctx->binfuncs);
  TCALC_VEC_FREE(ctx->unfuncs);
  TCALC_VEC_FREE(ctx->vars);
  TCALC_VEC_FREE(ctx->unops);
  TCALC_VEC_FREE(ctx->binops);
  TCALC_VEC_FREE(ctx->relops);
  TCALC_VEC_FREE(ctx->unlops);
  TCALC_VEC_FREE(ctx->binlops);

  free(ctx);
}

tcalc_err tcalc_ctx_addvar(tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_val val) {
  tcalc_err err = TCALC_ERR_OK;
  for (size_t i = 0; i < ctx->vars.len; i++) {
    if (tcalc_streq_ntlb(ctx->vars.arr[i].id, name, name_len)) {
      ctx->vars.arr[i].val = val;
      return TCALC_ERR_OK;
    }
  }

  tcalc_vardef def = { 0 };
  tcalc_strlcpy(def.id, name, TCALC_IDDEF_MAX_STR_SIZE);
  def.val = val;

  ret_on_macerr(err, TCALC_VEC_PUSH(ctx->vars, def, err));
  return TCALC_ERR_OK;
}

tcalc_err tcalc_ctx_addunfunc(tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_val_unfunc func) {
  tcalc_err err = TCALC_ERR_OK;
  for (size_t i = 0; i < ctx->unfuncs.len; i++) {
    if (tcalc_streq_ntlb(ctx->unfuncs.arr[i].id, name, name_len)) {
      ctx->unfuncs.arr[i].func = func;
      return TCALC_ERR_OK;
    }
  }

  tcalc_unfuncdef def = { 0 };
  tcalc_strlcpy(def.id, name, TCALC_IDDEF_MAX_STR_SIZE);
  def.func = func;
  ret_on_macerr(err, TCALC_VEC_PUSH(ctx->unfuncs, def, err));
  return TCALC_ERR_OK;
}

tcalc_err tcalc_ctx_addbinfunc(tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_val_binfunc func) {
  tcalc_err err = TCALC_ERR_OK;
  for (size_t i = 0; i < ctx->binfuncs.len; i++) {
    if (tcalc_streq_ntlb(ctx->binfuncs.arr[i].id, name, name_len)) {
      ctx->binfuncs.arr[i].func = func;
      return TCALC_ERR_OK;
    }
  }

  tcalc_binfuncdef def = { 0 };
  tcalc_strlcpy(def.id, name, TCALC_IDDEF_MAX_STR_SIZE);
  def.func = func;
  ret_on_macerr(err, TCALC_VEC_PUSH(ctx->binfuncs, def, err));
  return TCALC_ERR_OK;
}

#define tcalc_ctx_addxop(vec, opid, prec_, assoc_, funcptr, deftype) \
  tcalc_err err = TCALC_ERR_OK; \
  for (size_t i = 0; i < vec.len; i++) { \
    if (tcalc_streq_ntlb(vec.arr[i].id, opid, name_len)) { \
      vec.arr[i].prec = prec_; \
      vec.arr[i].assoc = assoc_; \
      vec.arr[i].func = funcptr; \
      return TCALC_ERR_OK; \
    } \
  } \
  deftype def = { 0 }; \
  tcalc_strlcpy(def.id, opid, TCALC_OPDEF_MAX_STR_SIZE); \
  def.prec = prec_; \
  def.assoc = assoc_; \
  def.func = funcptr; \
  ret_on_macerr(err, TCALC_VEC_PUSH(vec, def, err)); \
  return TCALC_ERR_OK;

tcalc_err tcalc_ctx_addunop(tcalc_ctx* ctx, const char* name, size_t name_len, int prec, tcalc_assoc assoc, tcalc_val_unfunc func) {
  tcalc_ctx_addxop(ctx->unops, name, prec, assoc, func, tcalc_unopdef);
}

tcalc_err tcalc_ctx_addbinop(tcalc_ctx* ctx, const char* name, size_t name_len, int prec, tcalc_assoc assoc, tcalc_val_binfunc func) {
  tcalc_ctx_addxop(ctx->binops, name, prec, assoc, func, tcalc_binopdef);
}

tcalc_err tcalc_ctx_addrelop(tcalc_ctx* ctx,const char* name, size_t name_len, int prec, tcalc_assoc assoc, tcalc_val_relfunc func) {
  tcalc_ctx_addxop(ctx->relops, name, prec, assoc, func, tcalc_relopdef);
}

tcalc_err tcalc_ctx_addunlop(tcalc_ctx* ctx, const char* name, size_t name_len, int prec, tcalc_assoc assoc, tcalc_val_unlfunc func) {
  tcalc_ctx_addxop(ctx->unlops, name, prec, assoc, func, tcalc_unlopdef);
}

tcalc_err tcalc_ctx_addbinlop(tcalc_ctx* ctx, const char* name, size_t name_len, int prec, tcalc_assoc assoc, tcalc_val_binlfunc func) {
  tcalc_ctx_addxop(ctx->binlops, name, prec, assoc, func, tcalc_binlopdef);
}

bool tcalc_ctx_hasid(const tcalc_ctx* ctx, const char* name, size_t name_len) {
  return tcalc_ctx_hasvar(ctx, name, name_len) || tcalc_ctx_hasfunc(ctx, name, name_len);
}

bool tcalc_ctx_hasfunc(const tcalc_ctx* ctx, const char* name, size_t name_len) {
  return tcalc_ctx_hasunfunc(ctx, name, name_len) || tcalc_ctx_hasbinfunc(ctx, name, name_len);
}

bool tcalc_ctx_hasop(const tcalc_ctx* ctx, const char* name, size_t name_len) {
 return tcalc_ctx_hasbinop(ctx, name, name_len) || tcalc_ctx_hasunop(ctx, name, name_len) ||
  tcalc_ctx_hasrelop(ctx, name, name_len) || tcalc_ctx_hasunlop(ctx, name, name_len) ||
  tcalc_ctx_hasbinlop(ctx, name, name_len);
}

bool tcalc_ctx_hasunfunc(const tcalc_ctx* ctx, const char* name, size_t name_len) {
  for (size_t i = 0; i < ctx->unfuncs.len; i++) {
    if (tcalc_streq_ntlb(ctx->unfuncs.arr[i].id, name, name_len))
      return true;
  }
  return false;
}

bool tcalc_ctx_hasbinfunc(const tcalc_ctx* ctx, const char* name, size_t name_len) {
  for (size_t i = 0; i < ctx->binfuncs.len; i++) {
    if (tcalc_streq_ntlb(ctx->binfuncs.arr[i].id, name, name_len))
      return true;
  }
  return false;
}

bool tcalc_ctx_hasvar(const tcalc_ctx* ctx, const char* name, size_t name_len) {
  for (size_t i = 0; i < ctx->vars.len; i++) {
    assert(ctx->vars.arr[i].id[TCALC_OPDEF_MAX_STR_SIZE - 1] == '\0');
    if (tcalc_streq_ntlb(ctx->vars.arr[i].id, name, name_len))
      return true;
  }
  return false;
}

bool tcalc_ctx_hasunop(const tcalc_ctx* ctx, const char* name, size_t name_len) {
  for (size_t i = 0; i < ctx->unops.len; i++) {
    if (tcalc_streq_ntlb(ctx->unops.arr[i].id, name, name_len))
      return true;
  }
  return false;
}

bool tcalc_ctx_hasbinop(const tcalc_ctx* ctx, const char* name, size_t name_len) {
  for (size_t i = 0; i < ctx->binops.len; i++) {
    if (tcalc_streq_ntlb(ctx->binops.arr[i].id, name, name_len))
      return true;
  }
  return false;
}

bool tcalc_ctx_hasrelop(const tcalc_ctx* ctx, const char* name, size_t name_len) {
  for (size_t i = 0; i < ctx->relops.len; i++) {
    if (tcalc_streq_ntlb(ctx->relops.arr[i].id, name, name_len))
      return true;
  }
  return false;
}

bool tcalc_ctx_hasunlop(const tcalc_ctx* ctx, const char* name, size_t name_len) {
  for (size_t i = 0; i < ctx->unlops.len; i++) {
    if (tcalc_streq_ntlb(ctx->unlops.arr[i].id, name, name_len))
      return true;
  }
  return false;
}

bool tcalc_ctx_hasbinlop(const tcalc_ctx* ctx, const char* name, size_t name_len) {
  for (size_t i = 0; i < ctx->binlops.len; i++) {
    if (tcalc_streq_ntlb(ctx->binlops.arr[i].id, name, name_len))
      return true;
  }
  return false;
}

tcalc_err tcalc_ctx_getvar(const tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_vardef* out) {
  for (size_t i = 0; i < ctx->vars.len; i++) {
    if (tcalc_streq_ntlb(ctx->vars.arr[i].id, name, name_len)) {
      *out = ctx->vars.arr[i];
      return TCALC_ERR_OK;
    }
  }
  return TCALC_ERR_NOT_FOUND;
}

tcalc_err tcalc_ctx_getunfunc(const tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_unfuncdef* out) {
  for (size_t i = 0; i < ctx->unfuncs.len; i++) {
    if (tcalc_streq_ntlb(ctx->unfuncs.arr[i].id, name, name_len)) {
      *out = ctx->unfuncs.arr[i];
      return TCALC_ERR_OK;
    }
  }
  return TCALC_ERR_NOT_FOUND;
}

tcalc_err tcalc_ctx_getbinfunc(const tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_binfuncdef* out) {
  for (size_t i = 0; i < ctx->binfuncs.len; i++) {
    if (tcalc_streq_ntlb(ctx->binfuncs.arr[i].id, name, name_len)) {
      *out = ctx->binfuncs.arr[i];
      return TCALC_ERR_OK;
    }
  }
  return TCALC_ERR_NOT_FOUND;
}

tcalc_err tcalc_ctx_getunop(const tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_unopdef* out) {
  for (size_t i = 0; i < ctx->unops.len; i++) {
    if (tcalc_streq_ntlb(ctx->unops.arr[i].id, name, name_len)) {
      *out = ctx->unops.arr[i];
      return TCALC_ERR_OK;
    }
  }
  return TCALC_ERR_NOT_FOUND;
}

tcalc_err tcalc_ctx_getbinop(const tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_binopdef* out) {
  for (size_t i = 0; i < ctx->binops.len; i++) {
    if (tcalc_streq_ntlb(ctx->binops.arr[i].id, name, name_len)) {
      *out = ctx->binops.arr[i];
      return TCALC_ERR_OK;
    }
  }
  return TCALC_ERR_NOT_FOUND;
}

tcalc_err tcalc_ctx_getrelop(const tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_relopdef* out) {
  for (size_t i = 0; i < ctx->relops.len; i++) {
    if (tcalc_streq_ntlb(ctx->relops.arr[i].id, name, name_len)) {
      *out = ctx->relops.arr[i];
      return TCALC_ERR_OK;
    }
  }
  return TCALC_ERR_NOT_FOUND;
}

tcalc_err tcalc_ctx_getunlop(const tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_unlopdef* out) {
  for (size_t i = 0; i < ctx->unlops.len; i++) {
    if (tcalc_streq_ntlb(ctx->unlops.arr[i].id, name, name_len)) {
      *out = ctx->unlops.arr[i];
      return TCALC_ERR_OK;
    }
  }
  return TCALC_ERR_NOT_FOUND;
}

tcalc_err tcalc_ctx_getbinlop(const tcalc_ctx* ctx, const char* name, size_t name_len, tcalc_binlopdef* out) {
  for (size_t i = 0; i < ctx->binlops.len; i++) {
    if (tcalc_streq_ntlb(ctx->binlops.arr[i].id, name, name_len)) {
      *out = ctx->binlops.arr[i];
      return TCALC_ERR_OK;
    }
  }
  return TCALC_ERR_NOT_FOUND;
}
