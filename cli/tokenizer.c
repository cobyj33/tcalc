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

  char** tokens;
  size_t returned_size;
  
  tcalc_error_t err = tcalc_tokenize_rpn(argv[1], &tokens, &returned_size);
  if (err) {
    printf("TCalc Error Occured: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < returned_size; i++) {
    printf("'%s', ", tokens[i]);
  }
  printf("%c", '\n');

  return EXIT_SUCCESS;
}