#include "tcalc_cli_progs.h"
#include "tcalc_cli_common.h"

#include "tcalc.h"

#include <stdio.h>
#include <stdlib.h>


#define TCALC_EXPRTREE_PRINT_MAX_DEPTH 20

void tcalc_exprtree_print(const char* expr, tcalc_exprtree* node, int depth);


int tcalc_cli_print_exprtree(const char* expr) {
  tcalc_ctx* ctx;
  tcalc_err err = tcalc_ctx_alloc_default(&ctx);
  TCALC_CLI_CHECK_ERR(err, "[%s] tcalc error: %s\n", __func__, tcalc_strerrcode(err));

  tcalc_exprtree* tree;
  err = tcalc_create_exprtree_infix(expr, ctx, &tree);
  tcalc_ctx_free(ctx);
  TCALC_CLI_CHECK_ERR(err, "[%s] tcalc error: %s\n", __func__, tcalc_strerrcode(err));

  tcalc_exprtree_print(expr, tree, 0);
  tcalc_exprtree_free(tree);
  return EXIT_SUCCESS;
}

void tcalc_exprtree_print(const char* expr, tcalc_exprtree* node, int depth) {
  if (node == NULL) return;

  for (int i = 0; i < depth; i++)
    fputs("|___", stdout);

  if (depth < TCALC_EXPRTREE_PRINT_MAX_DEPTH) {
    switch (node->type) {
      case TCALC_EXPRTREE_NODE_TYPE_BINARY: {
        printf("%.*s\n", TCALC_TOKEN_PRINTF_VARARG(expr, node->as.binary.token));
        tcalc_exprtree_print(expr, node->as.binary.left, depth + 1);
        tcalc_exprtree_print(expr, node->as.binary.right, depth + 1);
      } break;
      case TCALC_EXPRTREE_NODE_TYPE_UNARY: {
        printf("%.*s\n", TCALC_TOKEN_PRINTF_VARARG(expr, node->as.unary.token));
        tcalc_exprtree_print(expr, node->as.unary.child, depth + 1);
      } break;
      case TCALC_EXPRTREE_NODE_TYPE_VALUE: {
        printf("%.*s\n", TCALC_TOKEN_PRINTF_VARARG(expr, node->as.value.token));
      } break;
    }
  } else {
    fputs("...\n", stdout);
  }
}
