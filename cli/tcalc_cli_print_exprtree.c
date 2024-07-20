#include "tcalc_cli_progs.h"
#include "tcalc_cli_common.h"

#include "tcalc_error.h"
#include "tcalc_context.h"
#include "tcalc_exprtree.h"
#include "tcalc_tokens.h"
#include "tcalc_mac.h"

#include <stdio.h>
#include <stdlib.h>


#define TCALC_EXPRTREE_PRINT_MAX_DEPTH 20

void tcalc_exprtree_print(tcalc_exprtree* node, unsigned int depth);


int tcalc_cli_print_exprtree(const char* expr) {
  tcalc_ctx* ctx;
  tcalc_err err = tcalc_ctx_alloc_default(&ctx);
  TCALC_CLI_CHECK_ERR(err, "[%s] tcalc error: %s\n", FUNCDINFO, tcalc_strerrcode(err));

  tcalc_exprtree* tree;
  err = tcalc_create_exprtree_infix(expr, ctx, &tree);
  tcalc_ctx_free(ctx);
  TCALC_CLI_CHECK_ERR(err, "[%s] tcalc error: %s\n", FUNCDINFO, tcalc_strerrcode(err));

  tcalc_exprtree_print(tree, 0);
  tcalc_exprtree_free(tree);
  return EXIT_SUCCESS;
}

void tcalc_exprtree_print(tcalc_exprtree* node, unsigned int depth) {
  for (unsigned int i = 0; i < depth; i++)
    fputs("|___", stdout);

  if (depth < TCALC_EXPRTREE_PRINT_MAX_DEPTH) {
    printf("%s\n", node->token->val);

    for (size_t i = 0; i < node->nb_children; i++) {
      tcalc_exprtree_print(node->children[i], depth + 1);
    }
  } else {
    fputs("...\n", stdout);
  }
}
