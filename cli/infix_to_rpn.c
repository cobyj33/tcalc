#include "tcalc_mem.h"
#include "tcalc_tokens.h"
#include "tcalc_context.h"
#include "tcalc_constants.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>


int main(int argc, char** argv) {
  if (argc < 2) {
    printf("%s", "Insert 1 math string to be transformed to rpn \n");
    return EXIT_FAILURE;
  }

  tcalc_context_t* context;
  tcalc_error_t err = tcalc_context_alloc_default(&context);
  if (err) {
    printf("TCalc Error Occured: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
  }

  tcalc_token_t** infix_tokens;
  size_t nb_infix_tokens;
  err = tcalc_tokenize_infix(argv[1], &infix_tokens, &nb_infix_tokens);
  if (err) {
    printf("TCalc Error Occured: %s\n ", tcalc_strerrcode(err));
    tcalc_context_free(context);
    return EXIT_FAILURE;
  }


  tcalc_token_t** rpn_tokens;
  size_t nb_rpn_tokens;
  err = tcalc_infix_tokens_to_rpn_tokens(infix_tokens, nb_infix_tokens, context, &rpn_tokens, &nb_rpn_tokens);
  TCALC_ARR_FREE_F(infix_tokens, nb_infix_tokens, tcalc_token_free);
  tcalc_context_free(context);

  if (err) {
    printf("TCalc Error Occured: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < nb_rpn_tokens; i++) {
    printf("{type: %s, value: '%s'}, ", tcalc_token_type_get_string(rpn_tokens[i]->type),  rpn_tokens[i]->value);
  }
  printf("%s", "\n\n");

  for (size_t i = 0; i < nb_rpn_tokens; i++) {
    printf("'%s', ", rpn_tokens[i]->value);
  }
  printf("%c", '\n');


  TCALC_ARR_FREE_CF(rpn_tokens, nb_rpn_tokens, tcalc_token_free);
  return EXIT_SUCCESS;
}