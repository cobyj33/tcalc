#include "tcalc_cli_progs.h"
#include "tcalc_cli_common.h"

#include "tcalc_error.h"
#include "tcalc_tokens.h"
#include "tcalc_context.h"

#include <stdio.h>
#include <stdlib.h>


int tcalc_cli_infix_tokenizer(const char* expr) {
  tcalc_token** tokens;
  size_t nb_tokens;
  
  tcalc_err err = tcalc_tokenize_infix(expr, &tokens, &nb_tokens);
  if (err) {
    printf("TCalc Error Occured: %s\n ", tcalc_strerrcode(err));
    tcalc_errstk_printall();
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < nb_tokens; i++) {
    printf("{type: %s, value: '%s'}, ", tcalc_token_type_str(tokens[i]->type),  tokens[i]->val);
  }
  printf("%s", "\n\n");

  for (size_t i = 0; i < nb_tokens; i++) {
    printf("'%s'%s", tokens[i]->val, i == nb_tokens - 1 ? "" : ", ");
  }
  printf("%c", '\n');

  TCALC_ARR_FREE_F(tokens, nb_tokens, tcalc_token_free);
  return EXIT_SUCCESS;
}

