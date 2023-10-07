#include "tcalc_tokens.h"
#include "tcalc_string.h"
#include "tcalc_mem.h"
#include <stdlib.h>
#include <string.h>

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

/**
 * This function mainly serves to identify and resolve unary negative and positive signs
 * before they are processed by an actual parser.
 * 
 * Additionally, this tokenizer will make sure that parentheses are properly balanced
*/
tcalc_error_t tcalc_tokenize_infix(const char* expr, char*** out, size_t* returned_size) {
  *returned_size = 0;
  char** tokens;
  size_t nb_tokens;
  tcalc_error_t err = tcalc_tokenize(expr, &tokens, &nb_tokens);
  if (err) return err;

  #define NUM_OF_INDICATOR_TOKENS 6
  #define CLEAN_RETURN(pred, err) if (pred) { tcalc_free_arr((void**)tokens, nb_tokens, free); return err; }
  const char* indicator_tokens[NUM_OF_INDICATOR_TOKENS] = {"+", "-", "*", "/", "^", "("};

  CLEAN_RETURN(!tcalc_tokens_are_parentheses_balanced(tokens, nb_tokens), TCALC_INVALID_ARG)
  for (size_t i = 0; i < nb_tokens; i++) {
    if (strcmp(tokens[i], "-") != 0 && strcmp(tokens[i], "+") != 0)
      continue;

    // unary operation at the end of the string. This is an invalid expression.
    // while this is not a validation function, we do need a number placed after the unary operation to properly resolve the operation
    CLEAN_RETURN(i == nb_tokens - 1, TCALC_INVALID_ARG)

    int is_unary = i == 0 || has_in_strarr(indicator_tokens, NUM_OF_INDICATOR_TOKENS, tokens[i - 1]);
    if (!is_unary)
      continue;

    CLEAN_RETURN(!tcalc_strisdouble(tokens[i + 1]), TCALC_INVALID_ARG)

    if (strcmp(tokens[i], "-") == 0) {
      char* new_number_token = tcalc_strcombine("-", tokens[i + 1]);
      CLEAN_RETURN(new_number_token == NULL, TCALC_BAD_ALLOC)
      free(tokens[i + 1]);
      tokens[i + 1] = new_number_token;
    }
    free(tokens[i]); // free the operation string at the current index
    
    // shift eferything left
    for (size_t j = i; j < nb_tokens - 1; j++) {
      tokens[j] = tokens[j + 1];
    }
    nb_tokens--;
  }

  *out = tokens;
  *returned_size = nb_tokens;
  return err;
  #undef NUM_OF_INDICATOR_TOKENS
  #undef CLEAN_RETURN
}
