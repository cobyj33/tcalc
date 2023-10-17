#ifndef TCALC_CONTEXT_H
#define TCALC_CONTEXT_H

typedef enum {
  TCALC_RIGHT_ASSOCIATIVE,
  TCALC_LEFT_ASSOCIATIVE,
} tcalc_associativity_t;

typedef struct {
  int priority;
  tcalc_associativity_t associativity;
} tcalc_precedence_t;

typedef struct {
  char symbol;
  tcalc_precedence_t precedence;

} tcalc_binary_op_def_t;

typedef struct {
  char token;
} tcalc_unary_op_def_t;

typedef struct {
  tcalc_unary_op_def_t unary_ops;
  tcalc_binary_op_def_t binary_ops;
} tcalc_context_t;


#endif