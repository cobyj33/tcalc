#ifndef TCALC_VAL_H
#define TCALC_VAL_H

enum tcalc_valtype {
  TCALC_VALTYPE_NUM,
  TCALC_VALTYPE_BOOL
};

union tcalc_valunion {
  double num;
  int boolean;
};

const char* tcalc_valtype_str(enum tcalc_valtype type);

/**
 * A representation for any value returned or held by a value within tcalc.
 * A tcalc_val can only either represent a boolean value or a numerical value.
 */
typedef struct tcalc_val {
  enum tcalc_valtype type;
  union tcalc_valunion as;
} tcalc_val;

#define TCALC_VAL_INIT_NUM(_double_expr_) \
  (struct tcalc_val){ \
    .type = TCALC_VALTYPE_NUM, \
    .as = { \
      .num = (_double_expr_) \
    } \
  }

#define TCALC_VAL_INIT_BOOL(_bool_expr_) \
  (struct tcalc_val){ \
    .type = TCALC_VALTYPE_BOOL, \
    .as = { \
      .boolean = (_boolean_expr_) \
    } \
  }


#endif
