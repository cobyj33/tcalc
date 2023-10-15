#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "tcalc.h"
#include "tcalc_tokens.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("%s", "Insert 1 argument to be tokenized \n");
    return EXIT_FAILURE;
  }

  tcalc_token_t** tokens;
  size_t returned_size;
  
  tcalc_error_t err = tcalc_tokenize_rpn(argv[1], &tokens, &returned_size);
  if (err) {
    printf("TCalc Error Occured: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < returned_size; i++) {
    printf("{type: %s, value: '%s'}, ", tcalc_token_type_get_string(tokens[i]->type),  tokens[i]->value);
  }
  printf("%s", "\n\n");

  for (size_t i = 0; i < returned_size; i++) {
    printf("'%s', ", tokens[i]->value);
  }
  printf("%c", '\n');

  for (size_t i = 0; i < returned_size; i++) {
    tcalc_token_free(tokens[i]);
  }
  free(tokens);

  return EXIT_SUCCESS;
}