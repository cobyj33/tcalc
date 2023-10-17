#ifndef TCALC_CONTEXT_H
#define TCALC_CONTEXT_H

#include "tcalc_func_type.h"

typedef enum {
  TCALC_RIGHT_ASSOCIATIVE,
  TCALC_LEFT_ASSOCIATIVE,
} tcalc_associativity_t;

typedef struct {
  int priority;
  tcalc_associativity_t associativity;
} tcalc_precedence_t;

typedef struct {
  const char* identifier;
  const double value;
} tcalc_variable_def_t;

typedef struct {
  char symbol;
  tcalc_precedence_t precedence;
  tcalc_unary_func function;
} tcalc_unary_op_def_t;

typedef struct {
  char symbol;
  tcalc_precedence_t precedence;
  tcalc_binary_func function;
} tcalc_binary_op_def_t;

typedef struct {
  char start_symbol;
  char end_symbol;
} tcalc_grouping_symbol_def_t;

typedef struct {
  const char* identifier;
  tcalc_unary_func function;
} tcalc_unary_func_def_t;

typedef struct {
  const char* identifier;
  tcalc_binary_func function;
} tcalc_binary_func_def_t;

// these can be tweaked as they're needed. The general tcalc context
// should never really change
#define TCALC_CONTEXT_MAX_UNARY_FUNC_DEFS 16
#define TCALC_CONTEXT_MAX_BINARY_FUNC_DEFS 16
#define TCALC_CONTEXT_MAX_GROUPING_SYMBOL_DEFS 16
#define TCALC_CONTEXT_MAX_UNARY_OP_DEFS 16
#define TCALC_CONTEXT_MAX_BINARY_OP_DEFS 16
#define TCALC_CONTEXT_MAX_VARIABLE_DEFS 16

typedef struct {
  tcalc_unary_op_def_t unary_ops[TCALC_CONTEXT_MAX_UNARY_OP_DEFS];
  tcalc_binary_op_def_t binary_ops[TCALC_CONTEXT_MAX_BINARY_OP_DEFS];
  tcalc_grouping_symbol_def_t grouping_symbols[TCALC_CONTEXT_MAX_GROUPING_SYMBOL_DEFS];
  tcalc_unary_func_def_t unary_funcs[TCALC_CONTEXT_MAX_UNARY_FUNC_DEFS];
  tcalc_binary_func_def_t binary_funcs[TCALC_CONTEXT_MAX_BINARY_FUNC_DEFS];
  tcalc_variable_def_t binary_funcs[TCALC_CONTEXT_MAX_VARIABLE_DEFS];
} tcalc_context_t;


#endif