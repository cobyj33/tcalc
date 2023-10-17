
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
  int priority;
  tcalc_associativity_t associativity;
} tcalc_precedence_t;

typedef struct {
  tcalc_precedence_t precedence;
} tcalc_binary_op_def_t;

typedef struct {
  
} tcalc_unary_op_def_t;



typedef enum {
  TCALC_NUMBER,
  TCALC_UNARY_OPERATOR,
  TCALC_BINARY_OPERATOR,
  TCALC_FUNCTION,
  TCALC_GROUP_START,
  TCALC_GROUP_END
} tcalc_token_type_t;

const char* tcalc_token_type_get_string(tcalc_token_type_t token_type);

typedef struct {
  tcalc_token_type_t type;
  char* value;
} tcalc_token_t;

/**
 * 
 * Since tokens get passed around so much, it would be difficult to determine
 * which struct has ownership of a specific token to free its strings for example.
 * For that reason, all tokens should be allocated through the allocator and free
 * functions for tcalc_token_t types, so each time a token is used, we know it can
 * be freed safely
*/

/**
 * Because of the different contexts in which different operators can have in
 * different contexts and input formats (such as the ambiguous unary - + and binary - +),
 * we don't have a way to pass in a operator or number to allocation and automatically getting
 * a configured token.
*/
tcalc_error_t tcalc_token_alloc(tcalc_token_type_t type, char* value, tcalc_token_t** out);
tcalc_error_t tcalc_token_clone(tcalc_token_t* src, tcalc_token_t** out);
void tcalc_token_free(tcalc_token_t* token);
void tcalc_token_freev(void* token);

tcalc_error_t tcalc_tokenize_infix(const char* expr, tcalc_token_t*** out, size_t* out_size);
tcalc_error_t tcalc_tokenize_rpn(const char* expr, tcalc_token_t*** out, size_t* out_size);
tcalc_error_t tcalc_infix_tokens_to_rpn_tokens(tcalc_token_t** infix_tokens, size_t nb_infix_tokens, tcalc_token_t*** out, size_t* out_size);

int tcalc_is_valid_token_str(const char* token);

#endif