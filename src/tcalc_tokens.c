#include "tcalc_dstring.h"
#include "tcalc_darray.h"
#include "tcalc_error.h"
#include "tcalc_string.h"
#include "tcalc_tokens.h"
#include "tcalc_mem.h"

#include <stddef.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

const char* ALLOWED_CHARS = "0123456789. ()+-*/^%%";
const char* SINGLE_TOKENS = "()+-*/^%%";

int is_valid_tcalc_char(char ch);
tcalc_error_t tcalc_next_math_strtoken(const char* expr, char** out, size_t offset, size_t* new_offset);
int tcalc_tokens_are_parentheses_balanced(char** tokens, size_t nb_tokens);
tcalc_error_t tcalc_tokenize_strtokens(const char* expr, char*** out, size_t* out_size);

const char* tcalc_token_type_get_string(tcalc_token_type_t token_type) {
  switch (token_type) {
    case TCALC_NUMBER: return "number";
    case TCALC_UNARY_OPERATOR: return "unary operator";
    case TCALC_BINARY_OPERATOR: return "binary operator";
    case TCALC_FUNCTION: return "function";
    case TCALC_GROUP_START: return "group start";
    case TCALC_GROUP_END: return "group end";
  }
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



/**
 * This function mainly serves to identify and resolve unary negative and positive signs
 * before they are processed by an actual parser.
 * 
 * Additionally, this tokenizer will make sure that parentheses are properly balanced
*/
tcalc_error_t tcalc_tokenize_infix(const char* expr, tcalc_token_t*** out, size_t* out_size) {
  *out_size = 0;
  char** str_tokens;
  size_t nb_str_tokens;
  tcalc_error_t err = tcalc_tokenize_strtokens(expr, &str_tokens, &nb_str_tokens);
  if (err) return err;

  tcalc_token_t** infix_tokens = (tcalc_token_t**)malloc(sizeof(tcalc_token_t*) * nb_str_tokens);
  size_t nb_infix_tokens = 0;
  if (infix_tokens == NULL) {
    tcalc_free_arr((void**)str_tokens, nb_str_tokens, free);
    return TCALC_BAD_ALLOC;
  }

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

    } else if (strcmp(str_tokens[i], "*") == 0 || strcmp(str_tokens[i], "/") == 0 || strcmp(str_tokens[i], "^") == 0) {
      token_type = TCALC_BINARY_OPERATOR;
    } else if (strcmp(str_tokens[i], "(") == 0) {
      token_type = TCALC_GROUP_START;
    } else if (strcmp(str_tokens[i], ")") == 0) {
      token_type = TCALC_GROUP_END;
    } else if (tcalc_strisdouble(str_tokens[i])) {
      token_type = TCALC_NUMBER;
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
  tcalc_free_arr((void**)str_tokens, nb_str_tokens, free);
  return err;

  cleanup:
    tcalc_free_arr((void**)infix_tokens, nb_infix_tokens, tcalc_token_freev);
    tcalc_free_arr((void**)str_tokens, nb_str_tokens, free);
    return err;
}

/**
 * 
 * 
 * Checks for balanced parentheses too
*/
tcalc_error_t tcalc_tokenize_strtokens(const char* expr, char*** out, size_t* out_size) {
  tcalc_error_t err;
  *out_size = 0;
  tcalc_darray* token_buffer = tcalc_darray_alloc(sizeof(char*)); // char**
  if (token_buffer == NULL) return TCALC_BAD_ALLOC;

  size_t offset = 0;
  char* current_token;
  while ((err = tcalc_next_math_strtoken(expr, &current_token, offset, &offset)) == TCALC_OK) {
    if ((err = tcalc_darray_push(token_buffer, (void*)&current_token)) != TCALC_OK) {
      tcalc_darray_free_cb(token_buffer, free);
      return err;
    }
  }

  if (err != TCALC_STOP_ITER) {
    tcalc_darray_free_cb(token_buffer, free);
    return err;
  }

  if ((err = tcalc_darray_extract(token_buffer, (void**)out)) != TCALC_OK) {
    tcalc_darray_free_cb(token_buffer, free);
    return err;
  }

  if (!tcalc_tokens_are_parentheses_balanced(*out, tcalc_darray_size(token_buffer))) {
    tcalc_free_arr((void**)*out, tcalc_darray_size(token_buffer), free);
    tcalc_darray_free(token_buffer);
    return TCALC_UNBALANCED_GROUPING_SYMBOLS;
  }

  *out_size = tcalc_darray_size(token_buffer);
  tcalc_darray_free(token_buffer); // don't free with callback, as caller will have tokenized strings now
  return TCALC_OK;
}

tcalc_error_t tcalc_next_math_strtoken(const char* expr, char** out, size_t start, size_t* new_offset) {
  tcalc_error_t err;
  size_t offset = start;

	while (expr[offset] == ' ') // consume all spaces
		offset++;
  if (expr[offset] == '\0')
    return TCALC_STOP_ITER;

  if (!is_valid_tcalc_char(expr[offset])) 
    return TCALC_INVALID_ARG;

	for (int s = 0; SINGLE_TOKENS[s] != '\0'; s++) { // note that unary plus and minus will not work yet
		if (expr[offset] == SINGLE_TOKENS[s]) {
      if ((err = tcalc_strsubstr(expr, offset, offset + 1, out)) != TCALC_OK) return err;
      *new_offset = offset + 1;
      return TCALC_OK;
		}
	}

	if (isdigit(expr[offset]) || expr[offset] == '.') { // number checking
		if (expr[offset] == '.'  && !isdigit(expr[offset + 1])) { // lone decimal point
      return TCALC_INVALID_ARG;
		}

		int decimalCount = 0;
    size_t numstart = offset;

		while (expr[offset] != '\0' && (isdigit(expr[offset]) || expr[offset] == '.')) {
			if (expr[offset] == '.') {
				if (decimalCount > 0) return TCALC_INVALID_ARG;
				decimalCount++;
			}

      offset++;
		}
		
    if ((err = tcalc_strsubstr(expr, numstart, offset, out))  != TCALC_OK) return err; 
    *new_offset = offset;
    return TCALC_OK;
	}
	
	return TCALC_STOP_ITER;
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
    tcalc_free_arr((void**)token_strings, nb_str_tokens, free);
    return TCALC_BAD_ALLOC;
  }

  for (int i = 0; i < nb_str_tokens; i++) {
    if (!tcalc_is_valid_token_str(token_strings[i])) goto cleanup;

    tcalc_token_type_t token_type;

    if (strcmp(token_strings[i], "-") == 0 ||
        strcmp(token_strings[i], "+") == 0 || 
        strcmp(token_strings[i], "*") == 0 || 
        strcmp(token_strings[i], "/") == 0 || 
        strcmp(token_strings[i], "^") == 0) {
      token_type = TCALC_BINARY_OPERATOR;
    } else if (tcalc_strisdouble(token_strings[i])) {
      token_type = TCALC_NUMBER;
    } else { // Could not find matching token definition
      goto cleanup;
    }

    tcalc_token_t* token;
    if ((err = tcalc_token_alloc(token_type, token_strings[i], &token)) != TCALC_OK) {
      goto cleanup;
    }

    (*out)[i] = token;
    (*out_size)++;
  }

  *out_size = nb_str_tokens;
  tcalc_free_arr((void**)token_strings, nb_str_tokens, free);
  return TCALC_OK;

  cleanup:
    tcalc_free_arr((void**)token_strings, nb_str_tokens, free);
    tcalc_free_arr((void**)*out, *out_size, tcalc_token_freev); // strings in tokens are already freed by freeing the initial token strings
    return TCALC_INVALID_ARG;
}




typedef struct {
  tcalc_token_t token;
  int priority;
  tcalc_associativity_t associativity;
} tcalc_op_precedence_t;

tcalc_error_t tcalc_index_of_op_prec_data(const tcalc_op_precedence_t* operations, size_t nb_operations, tcalc_token_t* token, tcalc_op_precedence_t* out) {
  for (int i = 0; i < nb_operations; i++) {
    if (token->type == operations[i].token.type && strcmp(token->value, operations[i].token.value) == 0) {
      *out = operations[i];
      return TCALC_OK;
    }
  }
  return TCALC_NOT_FOUND;
}

/**
 * 
 * Remember that the number of infix tokens and the number of rpn tokens are not
 * necessarily the same, as rpn doesn't have grouping tokens at all
 * 
 * This isn't in tcalc_tokens because it's really just an implementation step for 
 * creating an expression tree
*/
tcalc_error_t tcalc_infix_tokens_to_rpn_tokens(tcalc_token_t** infix_tokens, size_t nb_infix_tokens, tcalc_token_t*** out, size_t* out_size) {
  tcalc_error_t err;
  #define OP_PRECEDENCE_DEF_COUNT 7

  const tcalc_op_precedence_t OP_PRECEDENCE_DEFS[OP_PRECEDENCE_DEF_COUNT] = {
    {{TCALC_BINARY_OPERATOR, "+"}, 1, TCALC_LEFT_ASSOCIATIVE},
    {{TCALC_BINARY_OPERATOR, "-"}, 1, TCALC_LEFT_ASSOCIATIVE},
    {{TCALC_BINARY_OPERATOR, "*"}, 2, TCALC_LEFT_ASSOCIATIVE},
    {{TCALC_BINARY_OPERATOR, "/"}, 2, TCALC_LEFT_ASSOCIATIVE},
    {{TCALC_BINARY_OPERATOR, "^"}, 3, TCALC_RIGHT_ASSOCIATIVE},
    {{TCALC_UNARY_OPERATOR, "+"}, 4, TCALC_RIGHT_ASSOCIATIVE},
    {{TCALC_UNARY_OPERATOR, "-"}, 4, TCALC_RIGHT_ASSOCIATIVE},
  };

  tcalc_token_t** operator_stack = (tcalc_token_t**)malloc(sizeof(tcalc_token_t*) * nb_infix_tokens);
  if (operator_stack == NULL) return TCALC_BAD_ALLOC;
  size_t operator_stack_size = 0;

  tcalc_token_t** rpn_tokens = (tcalc_token_t**)malloc(sizeof(tcalc_token_t*) * nb_infix_tokens); // tcalc_token_t* array, will be joined
  size_t rpn_tokens_size = 0;
  if (rpn_tokens == NULL) {
    free(operator_stack);
    return TCALC_BAD_ALLOC;
  }

  for (size_t i = 0; i < nb_infix_tokens; i++) {
    switch (infix_tokens[i]->type) {
      case TCALC_NUMBER: {
        tcalc_token_t* copy;
        if ((err = tcalc_token_clone(infix_tokens[i], &rpn_tokens[rpn_tokens_size])) != TCALC_OK) goto cleanup;
        rpn_tokens_size++;
        break;
      }
      case TCALC_GROUP_START: { // "("
        operator_stack[operator_stack_size++] = infix_tokens[i];
        break;
      }
      case TCALC_GROUP_END: { // ")"
        if (operator_stack_size == 0) {
          err = TCALC_INVALID_OP;
          goto cleanup;
        }

        while (strcmp(operator_stack[operator_stack_size - 1]->value, "(") != 0) { // keep popping onto output until the opening parenthesis is found
          if ((err = tcalc_token_clone(operator_stack[operator_stack_size - 1], &rpn_tokens[rpn_tokens_size])) != TCALC_OK) goto cleanup;
          rpn_tokens_size++;
          operator_stack_size--;
          if (operator_stack_size == 0) {
            err = TCALC_INVALID_OP;
            goto cleanup;
          }
        }
        operator_stack_size--; // pop off opening parenthesis

        break;
      }
      case TCALC_UNARY_OPERATOR:
      case TCALC_BINARY_OPERATOR: {
        tcalc_op_precedence_t current_optdef, stack_optdef;
        if ((err = tcalc_index_of_op_prec_data(OP_PRECEDENCE_DEFS, OP_PRECEDENCE_DEF_COUNT, infix_tokens[i], &current_optdef)) != TCALC_OK) goto cleanup;

        if (operator_stack_size > 0) {
          while (operator_stack_size > 0 && tcalc_index_of_op_prec_data(OP_PRECEDENCE_DEFS, OP_PRECEDENCE_DEF_COUNT, operator_stack[operator_stack_size - 1], &stack_optdef) == TCALC_OK) {
            if (stack_optdef.priority > current_optdef.priority || (stack_optdef.priority == current_optdef.priority && current_optdef.associativity == TCALC_LEFT_ASSOCIATIVE)) {
              if ((err = tcalc_token_clone(operator_stack[operator_stack_size - 1], &rpn_tokens[rpn_tokens_size])) != TCALC_OK) goto cleanup;
              rpn_tokens_size++;
              operator_stack_size--;
            } else { break; }
          }
        }

        operator_stack[operator_stack_size++] = infix_tokens[i];
        break;
      }
      default: {
        err = TCALC_UNIMPLEMENTED;
        goto cleanup;
      }
    }
  }

  while (operator_stack_size > 0) {
    if ((err = tcalc_token_clone(operator_stack[operator_stack_size - 1], &rpn_tokens[rpn_tokens_size])) != TCALC_OK) goto cleanup;
    rpn_tokens_size++;
    operator_stack_size--;
  }

  free(operator_stack);

  *out = rpn_tokens;
  *out_size = rpn_tokens_size;
  return TCALC_OK;

  cleanup:
   free(operator_stack);
    tcalc_free_arr((void**)rpn_tokens, rpn_tokens_size, tcalc_token_freev);
    *out_size = 0;
    return err;


  #undef OP_PRECEDENCE_DEF_COUNT
}





int is_valid_tcalc_char(char ch) {
	for (int i = 0; ALLOWED_CHARS[i] != '\0'; i++)
		if (ALLOWED_CHARS[i] == ch)
			return 1;
	return 0;
}

int tcalc_is_valid_token_str(const char* token) {
  if (token == NULL) return 0;
  if (token[0] == '\0') return 0; // empty string

  if (token[1] == '\0') { // single character string
    for (int i = 0; SINGLE_TOKENS[i] != '\0'; i++) {
      if (token[0] == SINGLE_TOKENS[i]) return 1;
    }
  }

  if (tcalc_strisdouble(token)) return 1;
  return 0;
}

int tcalc_tokens_are_parentheses_balanced(char** tokens, size_t nb_tokens) {
  int stack = 0;
  for (size_t i = 0; i < nb_tokens; i++) {
    if (strcmp(tokens[i], "(") == 0) {
      stack++;
    } else if (strcmp(tokens[i], ")") == 0) {
      stack--;
      if (stack < 0) return 0;
    }
  }

  return stack == 0;
}