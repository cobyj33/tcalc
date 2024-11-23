#include "tcalc.h"

#include <stdbool.h>
#include <stddef.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * ()[] - Grouping symbols
 * +-/%*^ - Operators
 * 0123456789 - digits
 * . - decimal point
 * , - parameter separator
 * abcdefghijklmnopqrstuvwxyz - function and variable names
*/
static const char* TCALC_ALLOWED_CHARS = ",()[]+-*/^%!=<>&|0123456789. abcdefghijklmnopqrstuvwxyz";
static const char* TCALC_SINGLE_TOKENS = ",()[]+-*/^%!=<>";
static const char* TCALC_MULTI_TOKENS[] = {"**", "==", "<=", ">=", "!=", "&&", "||", NULL}; // make sure this remains null terminated

static bool is_valid_tcalc_char(char ch);

static bool tcalc_token_ntstr_eq(
  const char* expr, struct tcalc_token token, const char* ntstr
) {
  return tcalc_streq_ntlb(ntstr, expr + token.start, token.xend - token.start);
}

static tcalc_err tcalc_next_math_strtoken(
  const char* expr, int32_t exprLen, int32_t req_start,
  int32_t* out_start, int32_t* out_xend
);

static tcalc_err tcalc_tokenize_infix_strtokens(
  const char* expr, int32_t exprLen, tcalc_token* destBuffer,
  int32_t destCapacity, int32_t* outDestLen
);

static tcalc_err tcalc_tokenize_infix_strtokens_assign_types(
  const char* expr, int32_t exprLen, tcalc_token* tokens, int32_t tokensLen
);

static bool tcalc_is_identifier(
  const char* source, int32_t sourceStart, int32_t sourceXEnd, int32_t checkLen
);

const char* tcalc_token_type_str(tcalc_token_type token_type) {
  switch (token_type) {
    case TCALC_TOK_NUM: return "number";
    case TCALC_TOK_UNOP: return "unary operator";
    case TCALC_TOK_BINOP: return "binary operator";
    case TCALC_TOK_ID: return "identifier";
    case TCALC_TOK_RELOP: return "relation operator";
    case TCALC_TOK_EQOP: return "equality operator";
    case TCALC_TOK_UNLOP: return "unary logical operator";
    case TCALC_TOK_BINLOP: return "binary logical operator";
    case TCALC_TOK_PSEP: return "parameter separator";
    case TCALC_TOK_GRPSTRT: return "group start";
    case TCALC_TOK_GRPEND: return "group end";
  }

  assert(0 && "unreachable");
  return "unknown token type";
}

bool tcalc_token_type_is_op(tcalc_token_type token_type)
{
  return token_type == TCALC_TOK_UNOP ||
    token_type == TCALC_TOK_BINOP ||
    token_type == TCALC_TOK_RELOP ||
    token_type == TCALC_TOK_UNLOP ||
    token_type == TCALC_TOK_BINLOP ||
    token_type == TCALC_TOK_EQOP;
}


tcalc_err tcalc_tokenize_infix(
  const char* expr,
  int32_t exprLen,
  tcalc_token* destBuffer,
  int32_t destCapacity,
  int32_t* outDestLength
) {
  *outDestLength = 0;
  int32_t destLength = 0;
  tcalc_err err = TCALC_ERR_OK;
  err = tcalc_tokenize_infix_strtokens(expr, exprLen, destBuffer, destCapacity, &destLength);
  if (err) return err;
  err = tcalc_tokenize_infix_strtokens_assign_types(expr, exprLen, destBuffer, destLength);
  *outDestLength = destLength;
  return err;
}


/**
 *
 * Tokenize infix expression and assign token types to each token.
 * Also inserts implicit multiplications where necessary
 *
 * tcalc_tokenize_infix checks for balanced grouping symbols and returns
 * TCALC_ERR_UNBAL_GRPSYMS error upon imbalanced grouping symbols.
 *
 *
 * tcalc_tokenize_infix serves to identify and resolve unary negative and positive signs
 * before they are processed.
 *
*/
static tcalc_err tcalc_tokenize_infix_strtokens_assign_types(
  const char* expr, int32_t exprLen, tcalc_token* tokens, int32_t tokensLen
) {
  tcalc_err err = TCALC_ERR_OK;
  for (int32_t i = 0; i < tokensLen && err == TCALC_ERR_OK; i++) {
    assert(tokens[i].xend > tokens[i].start); // none of these slices should be 0-length
    if (tcalc_token_ntstr_eq(expr, tokens[i], "+") ||
        tcalc_token_ntstr_eq(expr, tokens[i], "-")) {
      // + and - are unary if they are the first token in an expression,
      // or are precedded by a '(', ',', or some operator token

      if (i == 0 ||
          tokens[i - 1].type == TCALC_TOK_GRPSTRT ||
          tokens[i - 1].type == TCALC_TOK_PSEP ||
          tcalc_token_type_is_op(tokens[i - 1].type)) {
        tokens[i].type = TCALC_TOK_UNOP;
      } else {
        tokens[i].type = TCALC_TOK_BINOP;
      }
    } else if ( tcalc_token_ntstr_eq(expr, tokens[i], "*") ||
                tcalc_token_ntstr_eq(expr, tokens[i], "/") ||
                tcalc_token_ntstr_eq(expr, tokens[i], "^") ||
                tcalc_token_ntstr_eq(expr, tokens[i], "**") ||
                tcalc_token_ntstr_eq(expr, tokens[i], "%")) {
      tokens[i].type = TCALC_TOK_BINOP;
    } else if (tcalc_token_ntstr_eq(expr, tokens[i], "(")) {
      tokens[i].type = TCALC_TOK_GRPSTRT;
    } else if (tcalc_token_ntstr_eq(expr, tokens[i], ")")) {
      tokens[i].type = TCALC_TOK_GRPEND;
    } else if (tcalc_token_ntstr_eq(expr, tokens[i], "==") ||
              tcalc_token_ntstr_eq(expr, tokens[i], "=") ||
              tcalc_token_ntstr_eq(expr, tokens[i], "!=")) {
      tokens[i].type = TCALC_TOK_EQOP;
    } else if (tcalc_token_ntstr_eq(expr, tokens[i], "<") ||
              tcalc_token_ntstr_eq(expr, tokens[i], "<=") ||
              tcalc_token_ntstr_eq(expr, tokens[i], ">") ||
              tcalc_token_ntstr_eq(expr, tokens[i], ">=")) {
      tokens[i].type = TCALC_TOK_RELOP;
    } else if (tcalc_token_ntstr_eq(expr, tokens[i], "&&") ||
              tcalc_token_ntstr_eq(expr, tokens[i], "||")) {
      tokens[i].type = TCALC_TOK_BINLOP;
    } else if (tcalc_token_ntstr_eq(expr, tokens[i], "!")) {
      tokens[i].type = TCALC_TOK_UNLOP;
    } else if (tcalc_token_ntstr_eq(expr, tokens[i], ",")) {
      tokens[i].type = TCALC_TOK_PSEP;
    } else if (tcalc_lpstrisdouble(expr + tokens[i].start, tcalc_token_len(tokens[i]))) {
      tokens[i].type = TCALC_TOK_NUM;
    } else if (tcalc_is_identifier(expr, tokens[i].start, exprLen, tcalc_token_len(tokens[i]))) {
      tokens[i].type = TCALC_TOK_ID;
    } else { // could not identify token type, exit
      err = TCALC_ERR_INVALID_ARG;
    }
  }
  return err;
}

/**
 * Call and gather all tokens from subsequent calls to tcalc_next_math_strtoken
 * and return them in an array to *out.
 *
 * *out will be allocated with a size of *out_size by tcalc_tokenize_infix_strtokens
 * upon returning TCALC_ERR_OK. If TCALC_ERR_OK is not returned, then *out has not been
 * allocated and does not have to be freed.
 *
 * This function does NOT check for balanced parenthesis, or any other syntactical
 * errors in that matter. Some other examples of syntactical errors not checked
 * are unknown functions or variables.
 *
 * Lexical errors like miswritten numbers or unknown operators/symbols are seen
 * as errors, as according to tcalc_next_math_strtoken.
 *
 * Examples:
 *
 * "32+-34*(5 * 101)"
 * "32", "+", "-", "34", "*", "(", "5", "*", "101", ")"
 *
 * "3+sin(43)"
 * "3", "+", "sin", "(", "43", ")"
 *
 * "     [45 ^ (3 / 2)] *   +11"
 * "[", "45", "^", "(", "3", "/", "2", ")", "]", "*", "+", "11"
*/

static tcalc_err tcalc_tokenize_infix_strtokens(
  const char* expr, int32_t exprLen, tcalc_token* destBuffer,
  int32_t destCapacity, int32_t* outDestLen
) {
  *outDestLen = 0;
  int32_t nbTokensProcessed = 0;

  tcalc_err err = TCALC_ERR_OK;
  tcalc_token token = {0};

  while (nbTokensProcessed < destCapacity && err == TCALC_ERR_OK) {
    err = tcalc_next_math_strtoken(
      expr,
      exprLen,
      token.xend,
      &(token.start),
      &(token.xend)
    );
    if (err == TCALC_ERR_OK)
      destBuffer[nbTokensProcessed++] = token;
  }

  if (err == TCALC_ERR_STOP_ITER)
  {
    *outDestLen = nbTokensProcessed;
    err = TCALC_ERR_OK;
  }
  else
  {
    if (err == TCALC_ERR_OK)
      err = TCALC_ERR_NOMEM;
  }

  return err;
}

/**
 * Simple iterable tokenizing function
 *
 * Only the characters in the string "0123456789. abcdefghijklmnopqrstuvwxyz()[]+-*\/^%"
 * will be recognized (except the backslash ofc). Any other character being present will return an error.
 *
 * All +, -, /, *, ^, %, [, ,], (, ) tokens are immediately read and returned.
 * This even applies to negative numbers. A "-34" will be returned as two separate
 * tokens upon subsequent calls: "-" and "34"
 *
 * All strings of alphabetical lowercase letters will be grouped and returned as one.
 *
 * All whitespace is ignored
 *
 * TCALC_ERR_STOP_ITER is returned when the end of the expression has been reached.
 * If TCALC_ERR_STOP_ITER is returned, then *out is not allocated.
 *
 * TCALC_ERR_OK is returned when a token has been successfully read to *out.
 *
 * Any other error code is an actual lexing error and should be treated as such.
 *
 * This function should really never be called directly, and should be used through
 * tcalc_tokenize_infix_strtokens instead
 *
 * Examples:
 *
 * "32+-34*(5 * 101)"
 * "32", "+", "-", "34", "*", "(", "5", "*", "101", ")"
*/
static tcalc_err tcalc_next_math_strtoken(
  const char* expr, int32_t exprLen, int32_t req_start,
  int32_t* out_start, int32_t* out_xend
) {
  *out_start = req_start;
  *out_xend = req_start;
  int32_t start = req_start;

	while (start < exprLen && isblank(expr[start])) // consume all spaces
		start++;
  if (start >= exprLen)
    return TCALC_ERR_STOP_ITER;

  // Note that this doesn't catch all possible strings. For example, '&' is
  // an invalid token by itself, but will pass this point.
  if (!is_valid_tcalc_char(expr[start]))
    return TCALC_ERR_INVALID_ARG;

  *out_start = start;

  for (int s = 0; TCALC_MULTI_TOKENS[s] != NULL; s++) {
    if (tcalc_strhaspre(
        TCALC_MULTI_TOKENS[s], (int32_t)strlen(TCALC_MULTI_TOKENS[s]),
        expr + start, exprLen - start)
    ) {
      *out_xend = start + (int32_t)strlen(TCALC_MULTI_TOKENS[s]);
      return TCALC_ERR_OK;
    }
  }

	for (int s = 0; TCALC_SINGLE_TOKENS[s] != '\0'; s++) { // checking for operator and grouping symbols
		if (expr[start] == TCALC_SINGLE_TOKENS[s]) {
      *out_xend = start + 1;
      return TCALC_ERR_OK;
		}
	}


	if (tcalc_is_digit(expr[start]) || expr[start] == '.') { // number checking.
    // lone decimal point
		if (expr[start] == '.'  && (start + 1 >= exprLen || !tcalc_is_digit(expr[start + 1]))) {
      return TCALC_ERR_INVALID_ARG;
		}

		bool foundDecimal = false;
    int32_t xend = start;

		while (xend < exprLen && (tcalc_is_digit(expr[xend]) || expr[xend] == '.')) {
			if (expr[xend] == '.') {
				if (foundDecimal) return TCALC_ERR_INVALID_ARG;
        foundDecimal = true;
			}

      xend++;
		}

    *out_xend = xend;
    return TCALC_ERR_OK;
	}

  if (tcalc_is_lower(expr[start])) { // identifier checking
    int32_t xend = start;
    while (xend < exprLen && tcalc_is_lower(expr[xend]))
      xend++;
    *out_xend = xend;
    return TCALC_ERR_OK;
  }

	return TCALC_ERR_UNKNOWN_TOKEN;
}

static bool is_valid_tcalc_char(char ch) {
  int i = 0;
  while (TCALC_ALLOWED_CHARS[i] != '\0' && TCALC_ALLOWED_CHARS[i] != ch)
    i++;
  return TCALC_ALLOWED_CHARS[i] != '\0';
}

static bool tcalc_is_identifier(
  const char* source, int32_t sourceStart, int32_t sourceXEnd, int32_t checkLen
) {
  int32_t i = sourceStart;
  while (i < sourceXEnd && i < sourceStart + checkLen && tcalc_is_lower(source[i]))
    i++;
  return i == sourceStart + checkLen;
}
