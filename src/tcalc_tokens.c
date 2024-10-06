#include "tcalc_tokens.h"
#include "tcalc_error.h"
#include "tcalc_string.h"
#include "tcalc_mem.h"
#include "tcalc_context.h"
#include "tcalc_mac.h"

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
const char* TCALC_ALLOWED_CHARS = ",()[]+-*/^%!=<>&|0123456789. abcdefghijklmnopqrstuvwxyz";
const char* TCALC_SINGLE_TOKENS = ",()[]+-*/^%!=<>";
const char* TCALC_MULTI_TOKENS[] = {"**", "==", "<=", ">=", "!=", "&&", "||", NULL}; // make sure this remains null terminated

static bool is_valid_tcalc_char(char ch);
static tcalc_err tcalc_next_math_strtoken(const char* expr, size_t req_start, size_t* out_start, size_t* out_xend);
static bool tcalc_are_groupsyms_balanced(const char* expr);
static tcalc_err tcalc_tokenize_infix_strtokens(const char* expr, tcalc_slice** out, size_t* out_size, size_t* out_cap);
static tcalc_err tcalc_tokenize_infix_strtokens_assign_types(const char* expr, const tcalc_ctx* ctx, const tcalc_slice* str_tokens, const size_t nb_str_tokens, tcalc_token** out, size_t* out_size);
static bool tcalc_is_identifier(const char* source, size_t len);

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
    case TCALC_TOK_EOF: return "group end";
  }

  assert(0 && "unreachable");
  return "unknown token type";
}

tcalc_err tcalc_tokenize_infix(const char* expr, tcalc_token** out, size_t* out_size) {
  assert(expr != NULL);
  assert(out != NULL);
  assert(out_size != NULL);

  *out = NULL;
  *out_size = 0;
  tcalc_ctx* ctx = NULL;
  tcalc_err err = tcalc_ctx_alloc_default(&ctx);
  if (err) return err;

  err = tcalc_tokenize_infix_ctx(expr, ctx, out, out_size);
  tcalc_ctx_free(ctx);
  return err;
}

tcalc_err tcalc_tokenize_infix_ctx(const char* expr, const tcalc_ctx* ctx, tcalc_token** out, size_t* out_size) {
  assert(out != NULL);
  assert(ctx != NULL);
  assert(out != NULL);
  assert(out_size != NULL);

  tcalc_err err = TCALC_ERR_OK;
  *out = NULL;
  *out_size = 0;

  if (!tcalc_are_groupsyms_balanced(expr)) {
    tcalc_errstkaddf(FUNCDINFO, "Unbalanced grouping symbols");
    return err;
  }

  tcalc_slice* str_tokens = NULL;
  size_t nb_str_tokens = 0;
  size_t str_tokens_cap = 0;
  ret_on_err(err, tcalc_tokenize_infix_strtokens(expr, &str_tokens, &nb_str_tokens, &str_tokens_cap));

  tcalc_token* infix_tokens = NULL;
  size_t nb_infix_tokens = 0;
  err = tcalc_tokenize_infix_strtokens_assign_types(expr, ctx, str_tokens, nb_str_tokens, &infix_tokens, &nb_infix_tokens);
  free(str_tokens);

  *out = infix_tokens;
  *out_size = nb_infix_tokens;
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
static tcalc_err tcalc_tokenize_infix_strtokens_assign_types(const char* expr, const tcalc_ctx* ctx, const tcalc_slice* str_tokens, const size_t nb_str_tokens, tcalc_token** out, size_t* out_size) {
  assert(!(str_tokens == NULL && nb_str_tokens != 0));
  assert(out != NULL);
  assert(out_size != NULL);
  tcalc_err err = TCALC_ERR_OK;
  *out = NULL;
  *out_size = 0;

  // TODO: Maybe this should just dynamically grow or something instead of
  // defaulting to twice the size. (Twice the size to handle
  // implicit multiplication case)
  tcalc_token* infix_tokens = calloc(nb_str_tokens * 2, sizeof(*infix_tokens));
  size_t nb_infix_tokens = 0;
  if (infix_tokens == NULL) { return TCALC_ERR_BAD_ALLOC; }

  // arbitrary value. Since this is only used for implicit multiplication, just
  // start it on some tcalc_token_type which won't trigger an implicit
  // multiplication token.
  enum tcalc_token_type last_token_type = TCALC_TOK_PSEP;
  tcalc_slice last_slice = {0};

  for (size_t i = 0; i < nb_str_tokens; i++) {
    tcalc_token_type token_type;
    const tcalc_slice slice = str_tokens[i];
    assert(slice.xend > slice.start); // none of these slices should be 0-length

    if (tcalc_slice_ntstr_eq(expr, slice, "+") || tcalc_slice_ntstr_eq(expr, slice, "-")) {

      if (i == 0) { // + and - are unary if they are the first token in an expression
        token_type = TCALC_TOK_UNOP;
      } else if (infix_tokens[i - 1].type == TCALC_TOK_GRPSTRT) { // + and - are unary if they are the first token in a grouping symbol
        token_type = TCALC_TOK_UNOP;
      } else if (infix_tokens[i - 1].type == TCALC_TOK_BINOP) { // + and - are unary if they follow another binary operator
        token_type = TCALC_TOK_UNOP;
      } else if (infix_tokens[i - 1].type == TCALC_TOK_UNOP) { // + and - are unary if they follow another binary operator
        token_type = TCALC_TOK_UNOP;
      } else { // in any other case, + and - are binary
        token_type = TCALC_TOK_BINOP;
      }

    } else if ( tcalc_slice_ntstr_eq(expr, str_tokens[i], "*") ||
                tcalc_slice_ntstr_eq(expr, str_tokens[i], "/") ||
                tcalc_slice_ntstr_eq(expr, str_tokens[i], "^") ||
                tcalc_slice_ntstr_eq(expr, str_tokens[i], "**") ||
                tcalc_slice_ntstr_eq(expr, str_tokens[i], "%")) {
      token_type = TCALC_TOK_BINOP;
    } else if (tcalc_slice_ntstr_eq(expr, slice, "(")) {
      token_type = TCALC_TOK_GRPSTRT;
    } else if (tcalc_slice_ntstr_eq(expr, slice, ")")) {
      token_type = TCALC_TOK_GRPEND;
    } else if (tcalc_slice_ntstr_eq(expr, slice, "==") ||
              tcalc_slice_ntstr_eq(expr, slice, "=") ||
              tcalc_slice_ntstr_eq(expr, slice, "!=")) {
      token_type = TCALC_TOK_EQOP;
    } else if (tcalc_slice_ntstr_eq(expr, slice, "<") ||
              tcalc_slice_ntstr_eq(expr, slice, "<=") ||
              tcalc_slice_ntstr_eq(expr, slice, ">") ||
              tcalc_slice_ntstr_eq(expr, slice, ">=")) {
      token_type = TCALC_TOK_RELOP;
    } else if (tcalc_slice_ntstr_eq(expr, slice, "&&") ||
              tcalc_slice_ntstr_eq(expr, slice, "||")) {
      token_type = TCALC_TOK_BINLOP;
    } else if (tcalc_slice_ntstr_eq(expr, slice, "!")) {
      token_type = TCALC_TOK_UNLOP;
    } else if (tcalc_slice_ntstr_eq(expr, slice, ",")) {
      token_type = TCALC_TOK_PSEP;
    } else if (tcalc_lpstrisdouble(expr + slice.start, tcalc_slice_len(slice))) {
      token_type = TCALC_TOK_NUM;
    } else if (tcalc_is_identifier(expr + slice.start, tcalc_slice_len(slice))) {
      token_type = TCALC_TOK_ID;
    } else { // could not identify token type, exit
      return TCALC_ERR_INVALID_ARG;
    }


    // there is no way to make this code block look good.
    // detects if an implicit multiplication token should be inserted
    if (
      (
        last_token_type == TCALC_TOK_NUM ||
        last_token_type == TCALC_TOK_GRPEND ||
        (
          last_token_type == TCALC_TOK_ID &&
          tcalc_ctx_hasvar(ctx, expr + last_slice.start, tcalc_slice_len(last_slice))
        )
      ) &&
      (token_type == TCALC_TOK_GRPSTRT || token_type == TCALC_TOK_ID)
    ) {

      // Insert 0-length implicit multiplication token
      infix_tokens[nb_infix_tokens++] = (struct tcalc_token){
        .type = TCALC_TOK_BINOP,
        .start = slice.start,
        .xend = slice.start
      };
    }

    infix_tokens[nb_infix_tokens++] = (struct tcalc_token){
      .type = token_type,
      .start = slice.start,
      .xend = slice.xend
    };
    last_token_type = token_type;
    last_slice = slice;
  }

  *out = infix_tokens;
  *out_size = nb_infix_tokens;
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
static tcalc_err tcalc_tokenize_infix_strtokens(const char* expr, tcalc_slice** out, size_t* out_size, size_t* out_cap) {
  assert(expr != NULL);
  assert(out != NULL);
  assert(out_size != NULL);
  assert(out_cap != NULL);
  *out = NULL;
  *out_size = 0;
  *out_cap = 0;

  tcalc_err err = TCALC_ERR_OK;
  tcalc_slice* token_buffer = NULL;
  size_t tb_size = 0;
  size_t tb_capacity = 0;

  tcalc_slice slice = {0};
  while ((err = tcalc_next_math_strtoken(expr, slice.xend, &(slice.start), &(slice.xend))) == TCALC_ERR_OK) {
    cleanup_on_macerr(err, TCALC_DARR_PUSH(token_buffer, tb_size, tb_capacity, slice, err));
  }

  if (err != TCALC_ERR_STOP_ITER) goto cleanup;
  *out = token_buffer;
  *out_size = tb_size;
  *out_cap = tb_capacity;
  return TCALC_ERR_OK;

  cleanup:
    free(token_buffer);
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
static tcalc_err tcalc_next_math_strtoken(const char* expr, size_t req_start, size_t* out_start, size_t* out_xend) {
  assert(expr != NULL);
  assert(out_start != NULL);
  assert(out_xend != NULL);
  *out_start = req_start;
  *out_xend = req_start;

  size_t start = req_start;

	while (expr[start] == ' ') // consume all spaces
		start++;
  if (expr[start] == '\0')
    return TCALC_ERR_STOP_ITER;

  if (!is_valid_tcalc_char(expr[start]))
    return TCALC_ERR_INVALID_ARG;

  *out_start = start;

  for (size_t s = 0; TCALC_MULTI_TOKENS[s] != NULL; s++) {
    if (tcalc_strhaspre(TCALC_MULTI_TOKENS[s], expr + start)) {
      *out_xend = start + strlen(TCALC_MULTI_TOKENS[s]);
      return TCALC_ERR_OK;
    }
  }

	for (size_t s = 0; TCALC_SINGLE_TOKENS[s] != '\0'; s++) { // checking for operator and grouping symbols
		if (expr[start] == TCALC_SINGLE_TOKENS[s]) {
      *out_xend = start + 1;
      return TCALC_ERR_OK;
		}
	}


	if (isdigit(expr[start]) || expr[start] == '.') { // number checking.
		if (expr[start] == '.'  && !isdigit(expr[start + 1])) { // lone decimal point
      return TCALC_ERR_INVALID_ARG;
		}

		int decimalCount = 0;
    size_t xend = start;

		while (expr[xend] != '\0' && (isdigit(expr[xend]) || expr[xend] == '.')) {
			if (expr[xend] == '.') {
				if (decimalCount > 0) return TCALC_ERR_INVALID_ARG;
				decimalCount++;
			}

      xend++;
		}

    *out_xend = xend;
    return TCALC_ERR_OK;
	}

  if (islower(expr[start])) { // identifier checking
    size_t xend = start;
    while (expr[xend] != '\0' && islower(expr[xend])) {
      xend++;
    }

    *out_xend = xend;
    return TCALC_ERR_OK;
  }

  assert(0 && "unreachable");
	return TCALC_ERR_STOP_ITER; // this SHOULD be unreachable
}

static bool is_valid_tcalc_char(char ch) {
	for (int i = 0; TCALC_ALLOWED_CHARS[i] != '\0'; i++)
		if (TCALC_ALLOWED_CHARS[i] == ch)
			return true;
	return false;
}

/**
 * There's probably a better way to implement this but whatever
*/
static bool tcalc_are_groupsyms_balanced(const char* expr) {
  int nb_parens = 0;

  for (size_t i = 0; expr[i] != '\0' && nb_parens >= 0; i++) {
    nb_parens += expr[i] == '(';
    nb_parens -= expr[i] == ')';
  }

  return nb_parens == 0;
}

static bool tcalc_is_identifier(const char* str, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (!islower(str[i])) return false;
  }

  return true;
}
