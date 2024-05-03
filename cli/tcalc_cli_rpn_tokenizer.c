#include "tcalc_cli_progs.h"
#include "tcalc_cli_common.h"

#include "tcalc_error.h"
#include "tcalc_context.h"
#include "tcalc_tokens.h"
#include "tcalc_exprtree.h"
#include "tcalc_mac.h"

#include <stdio.h>
#include <stdlib.h>


int tcalc_cli_rpn_tokenizer(const char* expr) {
  tcalc_ctx* ctx = NULL;
  tcalc_token** infix_tokens = NULL;
  tcalc_token** rpn_tokens = NULL;
  size_t nb_infix_tokens = 0;
  size_t nb_rpn_tokens = 0;

  tcalc_err err = tcalc_ctx_alloc_default(&ctx);
  TCALC_CLI_CLEANUP_ERR(err, "[%s] Error while allocating tcalc ctx: %s\n", FUNCDINFO, tcalc_strerrcode(err))

  err = tcalc_tokenize_infix_ctx(expr, ctx, &infix_tokens, &nb_infix_tokens);
  TCALC_CLI_CLEANUP_ERR(err, "[%s] Error while tokenizing expr: %s\n", FUNCDINFO, tcalc_strerrcode(err))

  err = tcalc_infix_tokens_to_rpn_tokens(infix_tokens, nb_infix_tokens, ctx, &rpn_tokens, &nb_rpn_tokens);
  TCALC_CLI_CLEANUP_ERR(err, "[%s] Error while converting infix syntax to rpn syntax: %s\n", FUNCDINFO, tcalc_strerrcode(err))

  for (size_t i = 0; i < nb_rpn_tokens; i++) {
    printf("{type: %s, value: '%s'}%s",
      tcalc_token_type_str(rpn_tokens[i]->type), 
      rpn_tokens[i]->val,
      i == nb_rpn_tokens - 1 ? "" : ", ");
  }

  fputs("\n\n", stdout);

  for (size_t i = 0; i < nb_rpn_tokens; i++) {
    printf("'%s'%s", rpn_tokens[i]->val, i == nb_rpn_tokens - 1 ? "" : ", ");
  }
  fputs("\n", stdout);

  cleanup:
    TCALC_ARR_FREE_F(infix_tokens, nb_infix_tokens, tcalc_token_free);
    TCALC_ARR_FREE_F(rpn_tokens, nb_rpn_tokens, tcalc_token_free);
    tcalc_ctx_free(ctx);

    return err ? EXIT_FAILURE : EXIT_SUCCESS;
}
