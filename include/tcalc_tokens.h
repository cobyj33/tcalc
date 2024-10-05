#ifndef _TCALC_TOKENS_H_
#define _TCALC_TOKENS_H_

#include "tcalc_error.h"
#include "tcalc_string.h"
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

// typedef struct tcalc_token {
//   tcalc_token_type type;
//   char* val;
// } tcalc_token;


// Data that a token can contain:
// Type
// Starting Offset
// Ending Offset
// Instead of a pointer to the string, provide a starting and ending index.
// This would allow for the area around strings in the data to be easily
// displayed if necessary.
// Line data can just be calculated later by iterating over the source string
// if it is needed. Line data should not be needed unless an error occurs, so
// there's no good reason to store it.

typedef struct tcalc_token {
  tcalc_token_type type;
  size_t start;
  size_t xend;
} tcalc_token;


inline static const char* tcalc_token_startcp(const char* str, tcalc_token tok) { return str + tok.start; }
inline static char* tcalc_token_startp(char* str, tcalc_token tok) { return str + tok.start; }
inline static size_t tcalc_token_len(tcalc_token tok) { return tok.xend - tok.start; }

tcalc_err tcalc_tokenize_infix_ctx(const char* expr, const struct tcalc_ctx* ctx, tcalc_token** out, size_t* out_size);
tcalc_err tcalc_tokenize_infix(const char* expr, tcalc_token** out, size_t* out_size);

#define TCALC_TOKEN_IMPLICIT_MULT_PRINTF_STR ("*")

#define TCALC_TOKEN_IS_IMPLICIT_MULT(token) ((token).type == TCALC_TOK_BINOP && tcalc_token_len((token)) == 0)

// Must use %.*s with tokens. This macro simplifies adding the token length
// and string information into the variable arguments section of printf formats.
// Also, it makes sure that the length is casted to a integer, like it
// should be, which is easy to forget
//
// Handles printing implicit multiplication
#define TCALC_TOKEN_PRINTF_VARARG(expr, token) \
  (TCALC_TOKEN_IS_IMPLICIT_MULT(token) ? (int)TCALC_STRLIT_LEN(TCALC_TOKEN_IMPLICIT_MULT_PRINTF_STR) : (int)tcalc_token_len((token))), \
  (TCALC_TOKEN_IS_IMPLICIT_MULT(token) ? (TCALC_TOKEN_IMPLICIT_MULT_PRINTF_STR) : tcalc_token_startcp((expr), (token)))

// Must use %.*s with tokens. This macro simplifies adding the token length
// and string information into the variable arguments section of printf formats.
// Also, it makes sure that the length is casted to a integer, like it
// should be, which is easy to forget
//
// Implicit multiplication is not handled as a special case.
#define TCALC_TOKEN_PRINTF_VARARG_EXACT(expr, token) \
  (int)tcalc_token_len((token)), tcalc_token_startcp((expr), (token))

#endif
