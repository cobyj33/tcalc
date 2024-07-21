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

typedef struct tcalc_val {
  enum tcalc_valtype type;
  union tcalc_valunion as;
} tcalc_val;


#endif
