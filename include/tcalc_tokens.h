#ifndef _TCALC_TOKENS_H_
#define _TCALC_TOKENS_H_

#include "tcalc_error.h"
#include <stddef.h>

struct tcalc_ctx;

/*
Allowed TCalc Tokens:
All Alphanumeric Characters
"+-/%*^" operators
"()" Parentheses
"." decimal point
" " whitespace (will be ignored)

Tokens will be separated based on:
- strings of unsigned integers
- strings of unsigned decimal numbers
- strings of lowercase letters
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

I thought it best to give equality it's own operator type to simplify parsing.
Note that this also applies to !=

Valid Examples:

"(54+23) * 34"
["(", "54", "+", "23", ")", "*", "34"]

"2^(3+3.5)/3"
*/

typedef enum tcalc_token_type {
  TCALC_TOK_NUM, // A parsable number
  TCALC_TOK_UNOP, // unary operator ("+", "-")
  TCALC_TOK_BINOP, // binary operator ("+", "-", "*", "/", "^", "**")
  TCALC_TOK_RELOP, // relational (binary) operator ("<", "<=", ">", ">=")
  TCALC_TOK_UNLOP, // unary logical operator ("!")
  TCALC_TOK_BINLOP, // binary logical operator ("&&", "||")
  TCALC_TOK_EQOP, // equality operator ("==", "=", "!=")
  TCALC_TOK_PSEP, // parameter separator (",")
  TCALC_TOK_ID, // identifier ([a-zA-Z_][a-zA-Z0-9_]*)
  TCALC_TOK_GRPSTRT, // starting token for a pair of grouping symbols ("(")
  TCALC_TOK_GRPEND, // ending token for a pair of grouping symbols (")")
  TCALC_TOK_EOF // end of file
} tcalc_token_type;

const char* tcalc_token_type_str(tcalc_token_type token_type);

/**
 * Since tokens get passed around so much, it would be difficult to determine
 * which struct has ownership of a specific token. This is important in tracking
 * when to free a tokens string value. For that reason, all tokens should be
 * allocated through the allocator and free functions for tcalc_token types,
 * so each time a token is used, we know its can be freed safely without some other
 * token having a pointer to the same string value
*/
typedef struct tcalc_token {
  tcalc_token_type type;
  char* val;
} tcalc_token;

/**
 * Because of the different contexts in which different operators can have in
 * different contexts and input formats (such as the ambiguous unary - + and binary - +),
 * we don't have a way to pass in a operator or number to allocation and automatically getting
 * a configured token. Tokenizer functions themselves have to determine when a given token has
 * a specific meaning
*/
tcalc_err tcalc_token_alloc(tcalc_token_type type, char* val, tcalc_token** out);
tcalc_err tcalc_token_clone(tcalc_token* src, tcalc_token** out);
void tcalc_token_free(tcalc_token* token);


tcalc_err tcalc_tokenize_infix_ctx(const char* expr, const struct tcalc_ctx* ctx, tcalc_token*** out, size_t* out_size);
tcalc_err tcalc_tokenize_infix(const char* expr, tcalc_token*** out, size_t* out_size);
tcalc_err tcalc_tokenize_rpn(const char* expr, tcalc_token*** out, size_t* out_size);

#endif
