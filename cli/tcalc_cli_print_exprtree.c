#include "tcalc_cli_progs.h"
#include "tcalc_cli_common.h"

#include "tcalc.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#define TCALC_EXPRTREE_PRINT_MAX_DEPTH 20

static void tcalc_exprtree_fdump_preorder(
  FILE* file, const char* expr, tcalc_exprtree* treeBuf, int32_t treeBufLen,
  tcalc_token* tokenBuf, int32_t tokenBufLen, int32_t exprNodeInd, int depth
);

int tcalc_cli_print_exprtree(const char* expr, int32_t exprLen) {
  tcalc_ctx* ctx;
  tcalc_err err = tcalc_ctx_alloc_default(&ctx);
  TCALC_CLI_CHECK_ERR(err, "[%s] tcalc error while initializing tcalc_ctx: %s\n", __func__, tcalc_strerrcode(err));

  err = tcalc_lex_parse(
    expr, exprLen, globalTokenBuffer, globalTokenBufferCapacity,
    globalTreeNodeBuffer, globalTreeNodeBufferCapacity,
    &globalTokenBufferLen, &globalTreeNodeBufferLen,
    &globalTreeNodeBufferRootIndex
  );
  TCALC_CLI_CHECK_ERR(err, "[%s] tcalc error while lexing and parsing in tcalc_ctx: %s\n", __func__, tcalc_strerrcode(err));


  tcalc_ctx_free(ctx);
  TCALC_CLI_CHECK_ERR(err, "[%s] tcalc error: %s\n", __func__, tcalc_strerrcode(err));

  tcalc_exprtree_fdump_preorder(
    stdout, expr, globalTreeNodeBuffer, globalTreeNodeBufferLen, globalTokenBuffer,
    globalTokenBufferLen, globalTreeNodeBufferRootIndex, 0
  );
  return EXIT_SUCCESS;
}

static tcalc_token tcalc_token_from_binary_token_ind(
  tcalc_token* tokenBuf, int32_t tokenBufLen, int32_t binNodeTokenInd
) {
  if (binNodeTokenInd < 0)
  {
    assert(-binNodeTokenInd < tokenBufLen);
    return (tcalc_token){
      .type = TCALC_TOK_BINOP,
      .start = tokenBuf[-binNodeTokenInd].start,
      .xend = tokenBuf[-binNodeTokenInd].start,
    };
  }
  assert(binNodeTokenInd < tokenBufLen);
  return tokenBuf[binNodeTokenInd];
}

static void tcalc_exprtree_fdump_preorder_indent(int depth)
{
  for (int i = 0; i < depth && i < TCALC_EXPRTREE_PRINT_MAX_DEPTH; i++)
    fputs("|___", stdout);
}

static void tcalc_exprtree_fdump_preorder(
  FILE* file, const char* expr, tcalc_exprtree* treeBuf, int32_t treeBufLen,
  tcalc_token* tokenBuf, int32_t tokenBufLen, int32_t exprNodeInd, int depth
) {
  if (depth >= TCALC_EXPRTREE_PRINT_MAX_DEPTH)
  {
    tcalc_exprtree_fdump_preorder_indent(depth);
    fputs("...\n", stdout);
    return;
  }

  switch (treeBuf[exprNodeInd].type) {
    case TCALC_EXPRTREE_NODE_TYPE_BINARY:
    {
      tcalc_exprtree_fdump_preorder_indent(depth);

      fprintf(
        file, "%.*s\n",
        TCALC_TOKEN_PRINTF_VARARG(
          expr,
          tcalc_token_from_binary_token_ind(
            tokenBuf, tokenBufLen,
            treeBuf[exprNodeInd].as.binary.tokenIndOImplMult
          )
        )
      );

      tcalc_exprtree_fdump_preorder(
        file, expr, treeBuf, treeBufLen, tokenBuf,
        tokenBufLen, treeBuf[exprNodeInd].as.binary.leftTreeInd, depth + 1
      );

      tcalc_exprtree_fdump_preorder(
        file, expr, treeBuf, treeBufLen, tokenBuf,
        tokenBufLen, treeBuf[exprNodeInd].as.binary.rightTreeInd, depth + 1
      );
    }
    break;
    case TCALC_EXPRTREE_NODE_TYPE_UNARY:
    {
      tcalc_exprtree_fdump_preorder_indent(depth);

      fprintf(
        file, "%.*s\n",
        TCALC_TOKEN_PRINTF_VARARG(
          expr, tokenBuf[treeBuf[exprNodeInd].as.unary.tokenInd]
        )
      );

      tcalc_exprtree_fdump_preorder(
        file, expr, treeBuf, treeBufLen, tokenBuf,
        tokenBufLen, treeBuf[exprNodeInd].as.unary.childTreeInd, depth + 1
      );
    }
    break;
    case TCALC_EXPRTREE_NODE_TYPE_VALUE:
    {
      tcalc_exprtree_fdump_preorder_indent(depth);

      fprintf(
        file, "%.*s\n",
        TCALC_TOKEN_PRINTF_VARARG(
          expr, tokenBuf[treeBuf[exprNodeInd].as.value.tokenInd]
        )
      );
    }
    break;
    case TCALC_EXPRTREE_NODE_TYPE_FUNC: {
      tcalc_exprtree_fdump_preorder_indent(depth);

      fprintf(
        file, "%.*s\n",
        TCALC_TOKEN_PRINTF_VARARG(
          expr, tokenBuf[treeBuf[exprNodeInd].as.value.tokenInd]
        )
      );

      int32_t funcArgNodeInd = treeBuf[exprNodeInd].as.func.funcArgHeadInd;
      while (funcArgNodeInd >= 0)
      {
        assert(treeBuf[funcArgNodeInd].type == TCALC_EXPRTREE_NODE_TYPE_FUNCARG);
        tcalc_exprtree_fdump_preorder(
          file, expr, treeBuf, treeBufLen, tokenBuf,
          tokenBufLen, funcArgNodeInd, depth + 1
        );
        funcArgNodeInd = treeBuf[funcArgNodeInd].as.funcarg.nextArgInd;
      }
    }
    break;
    case TCALC_EXPRTREE_NODE_TYPE_FUNCARG:
    {
      tcalc_exprtree_fdump_preorder(
        file, expr, treeBuf, treeBufLen, tokenBuf,
        tokenBufLen, treeBuf[exprNodeInd].as.funcarg.exprInd, depth
      );
    }
    break;
  }
}

