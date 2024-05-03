#include "tcalc_tokens.h"
#include "tcalc_error.h"
#include "tcalc_string.h"
#include "tcalc_mem.h"
#include "tcalc_context.h"

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

int is_valid_tcalc_char(char ch);
tcalc_err tcalc_valid_token_str(const char* token);
tcalc_err tcalc_next_math_strtoken(const char* expr, char** out, size_t offset, size_t* new_offset);
int tcalc_are_groupsyms_balanced(const char* expr);
tcalc_err tcalc_tokenize_infix_strtokens(const char* expr, char*** out, size_t* out_size);
tcalc_err tcalc_tokenize_infix_strtokens_assign_types(char** str_tokens, size_t nb_str_tokens, tcalc_token*** out, size_t* out_size);
tcalc_err tcalc_tokenize_infix_token_insertions(tcalc_token** tokens, size_t nb_tokens, const tcalc_ctx* ctx, tcalc_token*** out, size_t* out_size);
int tcalc_is_identifier(const char* str);

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

  return "unknown token type";
}

tcalc_err tcalc_token_alloc(tcalc_token_type type, char* val, tcalc_token** out) {
  if (val == NULL) return TCALC_ERR_INVALID_ARG;

  tcalc_token* token = (tcalc_token*)malloc(sizeof(tcalc_token));
  if (token == NULL) return TCALC_ERR_BAD_ALLOC;

  token->type = type;
  if (tcalc_strdup(val, &token->val) != TCALC_ERR_OK) {
    free(token);
    return TCALC_ERR_BAD_ALLOC;
  }

  *out = token;
  return TCALC_ERR_OK;
}

void tcalc_token_free(tcalc_token* token) {
  if (token == NULL) return;
  free(token->val);
  free(token);
}


tcalc_err tcalc_token_clone(tcalc_token* src, tcalc_token** out) {
  return tcalc_token_alloc(src->type, src->val, out);
}

tcalc_err tcalc_tokenize_infix(const char* expr, tcalc_token*** out, size_t* out_size) {
  *out = NULL;
  *out_size = 0;
  tcalc_ctx* ctx = NULL;

  tcalc_err err = tcalc_ctx_alloc_default(&ctx);
  if (err) return err;
  err = tcalc_tokenize_infix_ctx(expr, ctx, out, out_size);
  tcalc_ctx_free(ctx);
  return err;
}

tcalc_err tcalc_tokenize_infix_ctx(const char* expr, const tcalc_ctx* ctx, tcalc_token*** out, size_t* out_size) {
  assert(out != NULL); assert(out_size != NULL);

  tcalc_err err = TCALC_ERR_OK;
  *out = NULL;
  *out_size = 0;

  ret_on_err(err, tcalc_are_groupsyms_balanced(expr));

  char** str_tokens;
  size_t nb_str_tokens;
  ret_on_err(err, tcalc_tokenize_infix_strtokens(expr, &str_tokens, &nb_str_tokens));

  tcalc_token** initial_infix_tokens;
  size_t nb_initial_infix_tokens;
  err = tcalc_tokenize_infix_strtokens_assign_types(str_tokens, nb_str_tokens, &initial_infix_tokens, &nb_initial_infix_tokens);
  TCALC_ARR_FREE_F(str_tokens, nb_str_tokens, free);
  if (err) return err;

  tcalc_token** resolved_infix_tokens;
  size_t nb_resolved_infix_tokens;
  err = tcalc_tokenize_infix_token_insertions(initial_infix_tokens, nb_initial_infix_tokens, ctx, &resolved_infix_tokens, &nb_resolved_infix_tokens);
  TCALC_ARR_FREE_F(initial_infix_tokens, nb_initial_infix_tokens, tcalc_token_free);
  if (err) return err;

  *out = resolved_infix_tokens;
  *out_size = nb_resolved_infix_tokens;
  return err;
}

/**
 * 
 * 
 * Responsibilities:
 * - Insert shorthand multiplication logic
 *   - Essentially, if a number preceeds an identifier or grouping symbol, append a multiplication token after that number
*/
tcalc_err tcalc_tokenize_infix_token_insertions(tcalc_token** tokens, size_t nb_tokens, const tcalc_ctx* ctx, tcalc_token*** out, size_t* out_size) {
  tcalc_err err = TCALC_ERR_OK;
  tcalc_token** fin_toks = NULL;
  size_t nb_fin_toks = 0;
  size_t fin_toks_cap = 0;

  for (size_t i = 0; i < nb_tokens; i++) {
    tcalc_token* clone;
    cleanup_on_err(err, tcalc_token_clone(tokens[i], &clone));
    cleanup_on_macerr(err, TCALC_DARR_PUSH(fin_toks, nb_fin_toks, fin_toks_cap, clone, err));

    if (i + 1 < nb_tokens &&
      (tokens[i]->type == TCALC_TOK_NUM || 
      tokens[i]->type == TCALC_TOK_GRPEND ||
      (tokens[i]->type == TCALC_TOK_ID && tcalc_ctx_hasvar(ctx, tokens[i]->val))) &&
      (tokens[i + 1]->type == TCALC_TOK_GRPSTRT ||
      tokens[i + 1]->type == TCALC_TOK_ID)) {
        tcalc_token* mult;
        cleanup_on_err(err, tcalc_token_alloc(TCALC_TOK_BINOP, "*", &mult));
        cleanup_on_macerr(err, TCALC_DARR_PUSH(fin_toks, nb_fin_toks, fin_toks_cap, mult, err));
    }
  }

  *out = fin_toks;
  *out_size = nb_fin_toks;
  return err;

  cleanup:
    TCALC_ARR_FREE_F(fin_toks, nb_fin_toks, tcalc_token_free);
    return err;
}

/**
 * 
 * Tokenize infix expression and assign token types to each token.
 * 
 * tcalc_tokenize_infix checks for balanced grouping symbols and returns
 * TCALC_ERR_UNBALANCED_GROUPING_SYMBOLS error upon imbalanced grouping symbols.
 * 
 * 
 * tcalc_tokenize_infix serves to identify and resolve unary negative and positive signs
 * before they are processed.
 * 
*/
tcalc_err tcalc_tokenize_infix_strtokens_assign_types(char** str_tokens, size_t nb_str_tokens, tcalc_token*** out, size_t* out_size) {
  tcalc_err err = TCALC_ERR_OK;
  // const char* relation_op_tokens[8] = {"=", "<", ">", "!", "==", "!=", ">=", "<="};
  
  tcalc_token** infix_tokens = (tcalc_token**)malloc(sizeof(tcalc_token*) * nb_str_tokens);
  size_t nb_infix_tokens = 0;
  if (infix_tokens == NULL) { return TCALC_ERR_BAD_ALLOC; }

  for (size_t i = 0; i < nb_str_tokens; i++) {
    tcalc_token_type token_type;

    if (strcmp(str_tokens[i], "+")  == 0 || strcmp(str_tokens[i], "-") == 0) {
      
      if (i == 0) { // + and - are unary if they are the first token in an expression
        token_type = TCALC_TOK_UNOP;
      } else if (infix_tokens[i - 1]->type == TCALC_TOK_GRPSTRT) { // + and - are unary if they are the first token in a grouping symbol
        token_type = TCALC_TOK_UNOP;
      } else if (infix_tokens[i - 1]->type == TCALC_TOK_BINOP) { // + and - are unary if they follow another binary operator
        token_type = TCALC_TOK_UNOP;
      } else if (infix_tokens[i - 1]->type == TCALC_TOK_UNOP) { // + and - are unary if they follow another binary operator
        token_type = TCALC_TOK_UNOP;
      } else { // in any other case, + and - are binary
        token_type = TCALC_TOK_BINOP;
      }

    } else if ( strcmp(str_tokens[i], "*") == 0 ||
                strcmp(str_tokens[i], "/") == 0 ||
                strcmp(str_tokens[i], "^") == 0 || 
                strcmp(str_tokens[i], "**") == 0 || 
                strcmp(str_tokens[i], "%") == 0) {
      token_type = TCALC_TOK_BINOP;
    } else if (strcmp(str_tokens[i], "(") == 0 || strcmp(str_tokens[i], "[") == 0) {
      token_type = TCALC_TOK_GRPSTRT;
    } else if (strcmp(str_tokens[i], ")") == 0 || strcmp(str_tokens[i], "]") == 0) {
      token_type = TCALC_TOK_GRPEND;
    } else if (strcmp(str_tokens[i], "==") == 0 ||
              strcmp(str_tokens[i], "=") == 0 ||
              strcmp(str_tokens[i], "!=") == 0) {
      token_type = TCALC_TOK_EQOP;
    } else if (strcmp(str_tokens[i], "<") == 0 ||
              strcmp(str_tokens[i], "<=") == 0 ||
              strcmp(str_tokens[i], ">") == 0 ||
              strcmp(str_tokens[i], ">=") == 0) {
      token_type = TCALC_TOK_RELOP;
    } else if (strcmp(str_tokens[i], "&&") == 0 ||
              strcmp(str_tokens[i], "||") == 0) {
      token_type = TCALC_TOK_BINLOP;
    } else if (strcmp(str_tokens[i], "!") == 0) {
      token_type = TCALC_TOK_UNLOP;
    } else if (strcmp(str_tokens[i], ",") == 0) {
      token_type = TCALC_TOK_PSEP;
    } else if (tcalc_strisdouble(str_tokens[i])) {
      token_type = TCALC_TOK_NUM;
    } else if (tcalc_is_identifier(str_tokens[i])) {
      token_type = TCALC_TOK_ID;  
    } else { // could not identify token type, exit
      err = TCALC_ERR_INVALID_ARG;
      goto cleanup;
    }

    tcalc_token* token;
    cleanup_on_err(err, tcalc_token_alloc(token_type, str_tokens[i], &token));
    infix_tokens[nb_infix_tokens++] = token;
  }

  *out = infix_tokens;
  *out_size = nb_infix_tokens;
  return err;

  cleanup:
    TCALC_ARR_FREE_F(infix_tokens, nb_infix_tokens, tcalc_token_free);
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
tcalc_err tcalc_tokenize_infix_strtokens(const char* expr, char*** out, size_t* out_size) {
  tcalc_err err = TCALC_ERR_OK;
  *out_size = 0;
  char** token_buffer = NULL;
  size_t tb_size = 0;
  size_t tb_capacity = 0;

  size_t offset = 0;
  char* current_token;
  while ((err = tcalc_next_math_strtoken(expr, &current_token, offset, &offset)) == TCALC_ERR_OK) {
    cleanup_on_macerr(err, TCALC_DARR_PUSH(token_buffer, tb_size, tb_capacity, current_token, err));
  }

  if (err != TCALC_ERR_STOP_ITER) goto cleanup;
  *out = token_buffer;
  *out_size = tb_size;
  return TCALC_ERR_OK;

  cleanup:
    TCALC_ARR_FREE_F(token_buffer, tb_size, free);
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
tcalc_err tcalc_next_math_strtoken(const char* expr, char** out, size_t start, size_t* new_offset) {
  tcalc_err err;
  size_t offset = start;

	while (expr[offset] == ' ') // consume all spaces
		offset++;
  if (expr[offset] == '\0')
    return TCALC_ERR_STOP_ITER;

  if (!is_valid_tcalc_char(expr[offset])) 
    return TCALC_ERR_INVALID_ARG;

  for (size_t s = 0; TCALC_MULTI_TOKENS[s] != NULL; s++) {
    if (tcalc_strhaspre(TCALC_MULTI_TOKENS[s], expr + offset)) {
      size_t mlsize = strlen(TCALC_MULTI_TOKENS[s]);
      ret_on_err(err, tcalc_strsubstr(expr, offset, offset + mlsize, out));
      *new_offset = offset + mlsize;
      return TCALC_ERR_OK;
    }
  }

	for (size_t s = 0; TCALC_SINGLE_TOKENS[s] != '\0'; s++) { // checking for operator and grouping symbols
		if (expr[offset] == TCALC_SINGLE_TOKENS[s]) {
      ret_on_err(err, tcalc_strsubstr(expr, offset, offset + 1, out));
      *new_offset = offset + 1;
      return TCALC_ERR_OK;
		}
	}

	if (isdigit(expr[offset]) || expr[offset] == '.') { // number checking.
		if (expr[offset] == '.'  && !isdigit(expr[offset + 1])) { // lone decimal point
      return TCALC_ERR_INVALID_ARG;
		}

		int decimalCount = 0;
    const size_t num_start = offset;

		while (expr[offset] != '\0' && (isdigit(expr[offset]) || expr[offset] == '.')) {
			if (expr[offset] == '.') {
				if (decimalCount > 0) return TCALC_ERR_INVALID_ARG;
				decimalCount++;
			}

      offset++;
		}
		
    ret_on_err(err, tcalc_strsubstr(expr, num_start, offset, out));
    *new_offset = offset;
    return TCALC_ERR_OK;
	}

  if (islower(expr[offset])) { // identifier checking
    const size_t id_start = offset;
    while (expr[offset] != '\0' && islower(expr[offset])) {
      offset++;
    }

    ret_on_err(err, tcalc_strsubstr(expr, id_start, offset, out));
    *new_offset = offset;
    return TCALC_ERR_OK;
  }
	
	return TCALC_ERR_STOP_ITER; // this SHOULD be unreachable
}

/**
 * 
 * 
 * @param out allocate and return a list of tcalc_token objects based on expr param
 * @param out_size 
*/
tcalc_err tcalc_tokenize_rpn(const char* expr, tcalc_token*** out, size_t* out_size) {
  *out_size = 0;
  char** token_strings;
  size_t nb_str_tokens;
  tcalc_err err = tcalc_strsplit(expr, ' ', &token_strings, &nb_str_tokens); // very simple :)
  if (err) return err;

  *out = (tcalc_token**)malloc(sizeof(tcalc_token*) * nb_str_tokens);
  if (*out == NULL) {
    TCALC_ARR_FREE_F(token_strings, nb_str_tokens, free);
    return TCALC_ERR_BAD_ALLOC;
  }

  for (size_t i = 0; i < nb_str_tokens; i++) {
    cleanup_on_err(err, tcalc_valid_token_str(token_strings[i]));

    tcalc_token_type token_type;

    if (strcmp(token_strings[i], "-") == 0 ||
        strcmp(token_strings[i], "+") == 0 || 
        strcmp(token_strings[i], "*") == 0 || 
        strcmp(token_strings[i], "/") == 0 || 
        strcmp(token_strings[i], "^") == 0 ||
        strcmp(token_strings[i], "%") == 0) {
      token_type = TCALC_TOK_BINOP;
    } else if (tcalc_strisdouble(token_strings[i])) {
      token_type = TCALC_TOK_NUM;
    } else if (tcalc_is_identifier(token_strings[i])) {
      token_type = TCALC_TOK_ID;
    } else { // Could not find matching token definition
      err = TCALC_ERR_UNKNOWN_TOKEN;
      goto cleanup;
    }

    tcalc_token* token;
    cleanup_on_err(err, tcalc_token_alloc(token_type, token_strings[i], &token));

    (*out)[i] = token;
    (*out_size)++;
  }

  TCALC_ARR_FREE_F(token_strings, nb_str_tokens, free);
  return TCALC_ERR_OK;

  cleanup:
    TCALC_ARR_FREE_F(token_strings, nb_str_tokens, free);
    TCALC_ARR_FREE_F(*out, *out_size, tcalc_token_free);
    return err;
}

int is_valid_tcalc_char(char ch) {
	for (int i = 0; TCALC_ALLOWED_CHARS[i] != '\0'; i++)
		if (TCALC_ALLOWED_CHARS[i] == ch)
			return 1;
	return 0;
}

/**
 * 
 * returns TCALC_ERR_OK on success and TCALC_ERR_INVALID_ARG on error.
*/
tcalc_err tcalc_valid_token_str(const char* token) {
  if (token == NULL) return TCALC_ERR_INVALID_ARG;
  if (token[0] == '\0') return TCALC_ERR_INVALID_ARG; // empty string

  for (size_t s = 0; TCALC_MULTI_TOKENS[s] != NULL; s++) {
    if (strcmp(TCALC_MULTI_TOKENS[s], token) == 0) return TCALC_ERR_OK;
  }

  for (int i = 0; TCALC_SINGLE_TOKENS[i] != '\0'; i++) {
    if (token[0] == TCALC_SINGLE_TOKENS[i]) return TCALC_ERR_OK;
  }

  if (tcalc_strisdouble(token)) return TCALC_ERR_OK;
  if (tcalc_is_identifier(token)) return TCALC_ERR_OK;

  return TCALC_ERR_INVALID_ARG;
}


/**
 * There's probably a better way to implement this but whatever
*/
tcalc_err tcalc_are_groupsyms_balanced(const char* expr) {
  tcalc_err err = TCALC_ERR_OK;
  
  char* stack = NULL;
  size_t stack_size = 0;
  size_t stack_capacity = 0;
  char corresponding[256];
  corresponding[')'] = '(';
  corresponding[']'] = '[';

  for (size_t i = 0; expr[i] != '\0'; i++) {
    switch (expr[i]) {
      case '(':
      case '[': {
        TCALC_DARR_PUSH(stack, stack_size, stack_capacity, expr[i], err);
        if (err) goto cleanup;
        break;
      }
      case ')':
      case ']': {
        cleanup_on_err(err, err_pred(stack_size == 0, TCALC_ERR_UNBALANCED_GROUPING_SYMBOLS));
        cleanup_on_err(err, err_pred(stack[stack_size - 1] != corresponding[(int)expr[i]], TCALC_ERR_UNBALANCED_GROUPING_SYMBOLS));
        stack_size--;
        break;
      }
    }
  }

  free(stack);
  return stack_size == 0 ? TCALC_ERR_OK : TCALC_ERR_UNBALANCED_GROUPING_SYMBOLS;

  cleanup:
    free(stack);
    return err;
}

int tcalc_is_identifier(const char* str) {
  int is_identifier = 1;
  for (size_t i = 0; str[i] != '\0'; i++) {
    if (!islower(str[i])) {
      is_identifier = 0;
      break;
    } 
  }

  return is_identifier;
}