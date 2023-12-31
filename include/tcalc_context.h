#ifndef TCALC_CONTEXT_H
#define TCALC_CONTEXT_H

#include "tcalc_func_type.h"
#include <stddef.h>


typedef struct tcalc_variable_def_t {
  const char* identifier;
  double value;
} tcalc_variable_def_t;

typedef enum tcalc_associativity_t{
  TCALC_RIGHT_ASSOCIATIVE,
  TCALC_LEFT_ASSOCIATIVE,
} tcalc_associativity_t;

typedef struct tcalc_precedence_t {
  int priority;
  tcalc_associativity_t associativity;
} tcalc_precedence_t;

typedef struct tcalc_unary_op_def_t {
  char symbol;
  tcalc_precedence_t precedence;
  tcalc_unary_func function;
} tcalc_unary_op_def_t;

typedef struct tcalc_binary_op_def_t {
  char symbol;
  tcalc_precedence_t precedence;
  tcalc_binary_func function;
} tcalc_binary_op_def_t;

typedef struct tcalc_unary_func_def_t {
  const char* identifier;
  tcalc_unary_func function;
} tcalc_unary_func_def_t;

typedef struct tcalc_binary_func_def_t {
  const char* identifier;
  tcalc_binary_func function;
} tcalc_binary_func_def_t;

typedef struct tcalc_grouping_symbol_def_t {
  char start_symbol;
  char end_symbol;
} tcalc_grouping_symbol_def_t;


// these can be tweaked as they're needed. I just gave them values
// based on general math symbols.
#define TCALC_CONTEXT_MAX_UNARY_OP_DEFS 4
#define TCALC_CONTEXT_MAX_BINARY_OP_DEFS 8
#define TCALC_CONTEXT_MAX_GROUPING_SYMBOL_DEFS 6
#define TCALC_CONTEXT_MAX_UNARY_FUNC_DEFS 64
#define TCALC_CONTEXT_MAX_BINARY_FUNC_DEFS 32
#define TCALC_CONTEXT_MAX_VARIABLE_DEFS 16

/**
 * Some conditions to a tcalc context:
 * 
 * Unary function identifiers must only contain alphabetical characters
 * Binary function identifiers must only contain alphabetical characters
 * Variable identifiers must only contain aphabetical characters
 * 
 * Symbols and identifiers must NOT be the same across the tcalc_context 
 * - The only exception to the above rule is that unary operators and binary operators
 *  can have the same symbol identifiers. This should be taken with salt though, as the tokenizer
 *  will disambiguate itself which + or - are unary and which + or - are binary. 
 *  If I had created math I wouldn't have created it like this.
*/
typedef struct tcalc_context_t {
  tcalc_unary_func_def_t unary_funcs[TCALC_CONTEXT_MAX_UNARY_FUNC_DEFS];
  size_t nb_unary_funcs;
  tcalc_binary_func_def_t binary_funcs[TCALC_CONTEXT_MAX_BINARY_FUNC_DEFS];
  size_t nb_binary_funcs;
  tcalc_variable_def_t variables[TCALC_CONTEXT_MAX_VARIABLE_DEFS];
  size_t nb_variables;
} tcalc_context_t;

extern const tcalc_context_t TCALC_GLOBAL_CONTEXT;

tcalc_error_t tcalc_context_has_unary_func(const tcalc_context_t* context, const char* name);
tcalc_error_t tcalc_context_has_binary_func(const tcalc_context_t* context, const char* name);
tcalc_error_t tcalc_context_has_variable(const tcalc_context_t* context, const char* name);

tcalc_error_t tcalc_context_get_unary_func(const tcalc_context_t* context, const char* name, tcalc_unary_func_def_t* out);
tcalc_error_t tcalc_context_get_binary_func(const tcalc_context_t* context, const char* name, tcalc_binary_func_def_t* out);
tcalc_error_t tcalc_context_get_variable(const tcalc_context_t* context, const char* name, tcalc_variable_def_t* out);

#endif