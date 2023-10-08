
#ifndef TCALC_TOKENS_H
#define TCALC_TOKENS_H
#include "tcalc_error.h"
#include <stddef.h>

/*
Allowed TCalc Tokens:
All Alphanumeric Characters
"+-/*%^" operators
"()" Parentheses
"." decimal point
" " whitespace (will be ignored)

Tokens will be separated based on:
- strings of numerical characters
- 
- operators
- grouping symbols

Whitespace will not be included in the actual token output, but can affect what
tokens are interpreted as separate. For example:

Strings of characters will be disambiguated by the actual parser for the tokens.
The way that the characters are disambiguated will be up to the parser, as a string
of characters could simultaneously represent

Since names of functions and variables will definitely be disambiguous if multiplication
is allowed to be represented through placing variables next to each other (xy),
all strings of alphanumeric characters and "." characters will remain untouched.


Valid Examples:

"(54+23) * 34"
["(", "54", "+", "23", ")", "*", "34"]

"2^(3+3.5)/3"
*/
typedef enum {
  TCALC_RIGHT_ASSOCIATIVE,
  TCALC_LEFT_ASSOCIATIVE,
} tcalc_associativity_t;

typedef struct {
  char* name;
  int nb_inputs;
} tcalc_func_def_t;

typedef struct {
  char* name;
  int priority;
  tcalc_associativity_t associativity;
} tcalc_operator_def_t;

typedef enum {
  TCALC_NUMBER,
  TCALC_UNARY_OPERATOR,
  TCALC_BINARY_OPERATOR,
  TCALC_FUNCTION,
  TCALC_GROUP_START,
  TCALC_GROUP_END
} tcalc_token_type_t;

typedef struct {
  tcalc_token_type_t type;
  char* value;
} tcalc_token_t;


tcalc_error_t tcalc_tokenize(const char* expr, char*** out, size_t* returned_size);
tcalc_error_t tcalc_tokenize_infix(const char* expr, char*** out, size_t* returned_size);
tcalc_error_t tcalc_tokenize_rpn(const char* expr, char*** out, size_t* returned_size);
int tcalc_is_valid_token(const char* token);

#endif