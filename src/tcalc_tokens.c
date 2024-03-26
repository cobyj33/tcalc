#include "tcalc_error.h"
#include "tcalc_string.h"
#include "tcalc_tokens.h"
#include "tcalc_mem.h"
#include "tcalc_context.h"

#include <stddef.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/**
 * ()[] - Grouping symbols
 * +-/%*^ - Operators
 * 0123456789 - digits
 * . - decimal point
 * , - parameter separator
 * abcdefghijklmnopqrstuvwxyz - function and variable names
*/
const char* TCALC_ALLOWED_CHARS = "0123456789. abcdefghijklmnopqrstuvwxyz,()[]+-*/^%";
const char* TCALC_SINGLE_TOKENS = ",()[]+-*/^%";

int is_valid_tcalc_char(char ch);
tcalc_error_t tcalc_valid_token_str(const char* token);
tcalc_error_t tcalc_next_math_strtoken(const char* expr, char** out, size_t offset, size_t* new_offset);
int tcalc_are_groupsyms_balanced(const char* expr);
tcalc_error_t tcalc_tokenize_infix_strtokens(const char* expr, char*** out, size_t* out_size);
tcalc_error_t tcalc_tokenize_infix_strtokens_assign_types(char** str_tokens, size_t nb_str_tokens, tcalc_token_t*** out, size_t* out_size);
tcalc_error_t tcalc_tokenize_infix_token_insertions(tcalc_token_t** tokens, size_t nb_tokens, tcalc_token_t*** out, size_t* out_size);
int tcalc_is_identifier(const char* str);

const char* tcalc_token_type_get_string(tcalc_token_type_t token_type) {
  switch (token_type) {
    case TCALC_NUMBER: return "number";
    case TCALC_UNARY_OPERATOR: return "unary operator";
    case TCALC_BINARY_OPERATOR: return "binary operator";
    case TCALC_IDENTIFIER: return "identifier";
    case TCALC_PARAM_SEPARATOR: return "parameter separator";
    case TCALC_GROUP_START: return "group start";
    case TCALC_GROUP_END: return "group end";
  }

  return "unknown token type";
}

tcalc_error_t tcalc_token_alloc(tcalc_token_type_t type, char* value, tcalc_token_t** out) {
  if (value == NULL) return TCALC_INVALID_ARG;

  tcalc_token_t* token = (tcalc_token_t*)malloc(sizeof(tcalc_token_t));
  if (token == NULL)
    return TCALC_BAD_ALLOC;

  token->type = type;
  
  if (tcalc_strdup(value, &token->value) != TCALC_OK) {
    free(token);
    return TCALC_BAD_ALLOC;
  }

  *out = token;
  return TCALC_OK;
}

void tcalc_token_free(tcalc_token_t* token) {
  free(token->value);
  free(token);
}

void tcalc_token_freev(void* token) {
  tcalc_token_free((tcalc_token_t*)token);
}

tcalc_error_t tcalc_token_clone(tcalc_token_t* src, tcalc_token_t** out) {
  return tcalc_token_alloc(src->type, src->value, out);
}

tcalc_error_t tcalc_tokenize_infix(const char* expr, tcalc_token_t*** out, size_t* out_size) {
  tcalc_error_t err = TCALC_OK;
  *out_size = 0;
  if ((err = tcalc_are_groupsyms_balanced(expr)) != TCALC_OK) return err;

  char** str_tokens;
  size_t nb_str_tokens;
  err = tcalc_tokenize_infix_strtokens(expr, &str_tokens, &nb_str_tokens);
  if (err) return err;

  tcalc_token_t** initial_infix_tokens;
  size_t nb_initial_infix_tokens;
  err = tcalc_tokenize_infix_strtokens_assign_types(str_tokens, nb_str_tokens, &initial_infix_tokens, &nb_initial_infix_tokens);
  TCALC_ARR_FREE_F(str_tokens, nb_str_tokens, free);
  if (err) return err;

  tcalc_token_t** resolved_infix_tokens;
  size_t nb_resolved_infix_tokens;
  err = tcalc_tokenize_infix_token_insertions(initial_infix_tokens, nb_initial_infix_tokens, &resolved_infix_tokens, &nb_resolved_infix_tokens);
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
tcalc_error_t tcalc_tokenize_infix_token_insertions(tcalc_token_t** tokens, size_t nb_tokens, tcalc_token_t*** out, size_t* out_size) {
  tcalc_error_t err = TCALC_OK;
  tcalc_token_t** final_tokens = NULL;
  size_t nb_final_tokens = 0;
  size_t final_tokens_capacity = 0;

  for (size_t i = 0; i < nb_tokens; i++) {
    tcalc_token_t* clone;
    if ((err = tcalc_token_clone(tokens[i], &clone)) != TCALC_OK) goto cleanup;
    TCALC_DARR_PUSH(final_tokens, nb_final_tokens, final_tokens_capacity, clone, err);
    if (err) goto cleanup;

    if (i + 1 < nb_tokens) {
      if (tokens[i]->type == TCALC_NUMBER || tokens[i]->type == TCALC_GROUP_END) {
        if (tokens[i + 1]->type == TCALC_GROUP_START || tokens[i + 1]->type == TCALC_IDENTIFIER || tokens[i + 1]->type == TCALC_GROUP_START) {
          tcalc_token_t* clone;
          if ((err = tcalc_token_alloc(TCALC_BINARY_OPERATOR, "*", &clone)) != TCALC_OK) goto cleanup;
          TCALC_DARR_PUSH(final_tokens, nb_final_tokens, final_tokens_capacity, clone, err);
          if (err) goto cleanup;
        }
      }
    }
  }

  *out = final_tokens;
  *out_size = nb_final_tokens;
  return err;

  cleanup:
    TCALC_ARR_FREE_F(final_tokens, nb_final_tokens, tcalc_token_free);
    return err;
}

/**
 * 
 * Tokenize infix expression and assign token types to each token.
 * 
 * tcalc_tokenize_infix checks for balanced grouping symbols and returns
 * TCALC_UNBALANCED_GROUPING_SYMBOLS error upon imbalanced grouping symbols.
 * 
 * 
 * tcalc_tokenize_infix serves to identify and resolve unary negative and positive signs
 * before they are processed.
 * 
*/
tcalc_error_t tcalc_tokenize_infix_strtokens_assign_types(char** str_tokens, size_t nb_str_tokens, tcalc_token_t*** out, size_t* out_size) {
  tcalc_error_t err = TCALC_OK;
  
  tcalc_token_t** infix_tokens = (tcalc_token_t**)malloc(sizeof(tcalc_token_t*) * nb_str_tokens);
  size_t nb_infix_tokens = 0;
  if (infix_tokens == NULL) { return TCALC_BAD_ALLOC; }

  for (size_t i = 0; i < nb_str_tokens; i++) {
    tcalc_token_type_t token_type;

    if (strcmp(str_tokens[i], "+")  == 0 || strcmp(str_tokens[i], "-") == 0) {
      
      if (i == 0) { // + and - are unary if they are the first token in an expression
        token_type = TCALC_UNARY_OPERATOR;
      } else if (infix_tokens[i - 1]->type == TCALC_GROUP_START) { // + and - are unary if they are the first token in a grouping symbol
        token_type = TCALC_UNARY_OPERATOR;
      } else if (infix_tokens[i - 1]->type == TCALC_BINARY_OPERATOR) { // + and - are unary if they follow another binary operator
        token_type = TCALC_UNARY_OPERATOR;
      } else if (infix_tokens[i - 1]->type == TCALC_UNARY_OPERATOR) { // + and - are unary if they follow another binary operator
        token_type = TCALC_UNARY_OPERATOR;
      } else { // in any other case, + and - are binary
        token_type = TCALC_BINARY_OPERATOR;
      }

    } else if ( strcmp(str_tokens[i], "*") == 0 ||
                strcmp(str_tokens[i], "/") == 0 ||
                strcmp(str_tokens[i], "^") == 0 || 
                strcmp(str_tokens[i], "%") == 0) {
      token_type = TCALC_BINARY_OPERATOR;
    } else if (strcmp(str_tokens[i], ",") == 0) {
      token_type = TCALC_PARAM_SEPARATOR;
    } else if (strcmp(str_tokens[i], "(") == 0 || strcmp(str_tokens[i], "[") == 0) {
      token_type = TCALC_GROUP_START;
    } else if (strcmp(str_tokens[i], ")") == 0 || strcmp(str_tokens[i], "]") == 0) {
      token_type = TCALC_GROUP_END;
    } else if (tcalc_strisdouble(str_tokens[i])) {
      token_type = TCALC_NUMBER;
    } else if (tcalc_is_identifier(str_tokens[i])) {
      token_type = TCALC_IDENTIFIER;  
    } else { // could not identify token type, exit
      err = TCALC_INVALID_ARG;
      goto cleanup;
    }

    tcalc_token_t* token;
    if ((err = tcalc_token_alloc(token_type, str_tokens[i], &token)) != TCALC_OK) goto cleanup;
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
 * upon returning TCALC_OK. If TCALC_OK is not returned, then *out has not been
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
tcalc_error_t tcalc_tokenize_infix_strtokens(const char* expr, char*** out, size_t* out_size) {
  tcalc_error_t err = TCALC_OK;
  *out_size = 0;
  char** token_buffer = NULL;
  size_t tb_size = 0;
  size_t tb_capacity = 0;

  size_t offset = 0;
  char* current_token;
  while ((err = tcalc_next_math_strtoken(expr, &current_token, offset, &offset)) == TCALC_OK) {
    TCALC_DARR_PUSH(token_buffer, tb_size, tb_capacity, current_token, err);
    if (err) goto cleanup;
  }

  if (err != TCALC_STOP_ITER) goto cleanup;
  *out = token_buffer;
  *out_size = tb_size;
  return TCALC_OK;

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
 * TCALC_STOP_ITER is returned when the end of the expression has been reached.
 * If TCALC_STOP_ITER is returned, then *out is not allocated.
 * 
 * TCALC_OK is returned when a token has been successfully read to *out.
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
tcalc_error_t tcalc_next_math_strtoken(const char* expr, char** out, size_t start, size_t* new_offset) {
  tcalc_error_t err;
  size_t offset = start;

	while (expr[offset] == ' ') // consume all spaces
		offset++;
  if (expr[offset] == '\0')
    return TCALC_STOP_ITER;

  if (!is_valid_tcalc_char(expr[offset])) 
    return TCALC_INVALID_ARG;

	for (int s = 0; TCALC_SINGLE_TOKENS[s] != '\0'; s++) { // checking for operator and grouping symbols
		if (expr[offset] == TCALC_SINGLE_TOKENS[s]) {
      if ((err = tcalc_strsubstr(expr, offset, offset + 1, out)) != TCALC_OK) return err;
      *new_offset = offset + 1;
      return TCALC_OK;
		}
	}

	if (isdigit(expr[offset]) || expr[offset] == '.') { // number checking.
		if (expr[offset] == '.'  && !isdigit(expr[offset + 1])) { // lone decimal point
      return TCALC_INVALID_ARG;
		}

		int decimalCount = 0;
    const size_t num_start = offset;

		while (expr[offset] != '\0' && (isdigit(expr[offset]) || expr[offset] == '.')) {
			if (expr[offset] == '.') {
				if (decimalCount > 0) return TCALC_INVALID_ARG;
				decimalCount++;
			}

      offset++;
		}
		
    if ((err = tcalc_strsubstr(expr, num_start, offset, out))  != TCALC_OK) return err; 
    *new_offset = offset;
    return TCALC_OK;
	}

  if (islower(expr[offset])) { // identifier checking
    const size_t id_start = offset;
    while (expr[offset] != '\0' && islower(expr[offset])) {
      offset++;
    }

    if ((err = tcalc_strsubstr(expr, id_start, offset, out))  != TCALC_OK) return err; 
    *new_offset = offset;
    return TCALC_OK;
  }
	
	return TCALC_STOP_ITER; // this SHOULD be unreachable
}

/**
 * 
 * 
 * @param out allocate and return a list of tcalc_token_t objects based on expr param
 * @param out_size 
*/
tcalc_error_t tcalc_tokenize_rpn(const char* expr, tcalc_token_t*** out, size_t* out_size) {
  *out_size = 0;
  char** token_strings;
  size_t nb_str_tokens;
  tcalc_error_t err = tcalc_strsplit(expr, ' ', &token_strings, &nb_str_tokens); // very simple :)
  if (err) return err;

  *out = (tcalc_token_t**)malloc(sizeof(tcalc_token_t*) * nb_str_tokens);
  if (*out == NULL) {
    TCALC_ARR_FREE_F(token_strings, nb_str_tokens, free);
    return TCALC_BAD_ALLOC;
  }

  for (size_t i = 0; i < nb_str_tokens; i++) {
    if ((err = tcalc_valid_token_str(token_strings[i])) != TCALC_OK) goto cleanup;

    tcalc_token_type_t token_type;

    if (strcmp(token_strings[i], "-") == 0 ||
        strcmp(token_strings[i], "+") == 0 || 
        strcmp(token_strings[i], "*") == 0 || 
        strcmp(token_strings[i], "/") == 0 || 
        strcmp(token_strings[i], "^") == 0 ||
        strcmp(token_strings[i], "%") == 0) {
      token_type = TCALC_BINARY_OPERATOR;
    } else if (tcalc_strisdouble(token_strings[i])) {
      token_type = TCALC_NUMBER;
    } else if (tcalc_is_identifier(token_strings[i])) {
      token_type = TCALC_IDENTIFIER;
    } else { // Could not find matching token definition
      err = TCALC_UNKNOWN_TOKEN;
      goto cleanup;
    }

    tcalc_token_t* token;
    if ((err = tcalc_token_alloc(token_type, token_strings[i], &token)) != TCALC_OK) {
      goto cleanup;
    }

    (*out)[i] = token;
    (*out_size)++;
  }

  TCALC_ARR_FREE_F(token_strings, nb_str_tokens, free);
  return TCALC_OK;

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
 * returns TCALC_OK on success and TCALC_INVALID_ARG on error.
*/
tcalc_error_t tcalc_valid_token_str(const char* token) {
  if (token == NULL) return TCALC_INVALID_ARG;
  if (token[0] == '\0') return TCALC_INVALID_ARG; // empty string

  if (token[1] == '\0') { // single character string
    for (int i = 0; TCALC_SINGLE_TOKENS[i] != '\0'; i++) {
      if (token[0] == TCALC_SINGLE_TOKENS[i]) return TCALC_OK;
    }
  }

  if (tcalc_strisdouble(token)) return TCALC_OK;
  if (tcalc_is_identifier(token)) return TCALC_OK;

  return TCALC_INVALID_ARG;
}


/**
 * There's probably a better way to implement this but whatever
*/
tcalc_error_t tcalc_are_groupsyms_balanced(const char* expr) {
  tcalc_error_t err = TCALC_OK;
  
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
        if (stack_size == 0) {
          err = TCALC_UNBALANCED_GROUPING_SYMBOLS;
          goto cleanup;
        }

        if (stack[stack_size - 1] != corresponding[(int)expr[i]]) {
          err = TCALC_UNBALANCED_GROUPING_SYMBOLS;
          goto cleanup;
        }

        stack_size--;
        break;
      }
    }
  }

  free(stack);
  return stack_size == 0 ? TCALC_OK : TCALC_UNBALANCED_GROUPING_SYMBOLS;

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