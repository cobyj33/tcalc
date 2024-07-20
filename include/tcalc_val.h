#ifndef TCALC_VAL_H
#define TCALC_VAL_H

typedef enum tcalc_valtype {
  TCALC_TOK_NUM,
  TCALC_BOOLEAN
} tcalc_exprtype;

typedef struct tcalc_val {
  tcalc_exprtype type;
  union {
    double num;
    int boolean;
  } val;
} tcalc_val;


#endif
