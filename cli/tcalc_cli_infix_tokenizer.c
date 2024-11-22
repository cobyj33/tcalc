#include "tcalc_cli_progs.h"
#include "tcalc_cli_common.h"

#include "tcalc.h"

#include <stdio.h>
#include <stdlib.h>

int tcalc_cli_infix_tokenizer(const char* expr, int32_t exprLen) {
  tcalc_err err = tcalc_tokenize_infix(expr, exprLen, globalTokenBuffer, globalTokenBufferCapacity, &globalTokenBufferLen);
  TCALC_CLI_CHECK_ERR(err, "[%s] tcalc error: %s\n ", __func__, tcalc_strerrcode(err));

  for (int32_t i = 0; i < globalTokenBufferLen; i++) {
    fprintf(
      stdout,
      "{ type: %s, value: '%.*s' }, ",
      tcalc_token_type_str(globalTokenBuffer[i].type),
      TCALC_TOKEN_PRINTF_VARARG(expr, globalTokenBuffer[i])
    );
  }
  fputc('\n', stdout);
  fputs("--------------------------------", stdout);
  fputc('\n', stdout);

  for (int32_t i = 0; i < globalTokenBufferLen; i++) {
    fprintf(
      stdout,
      "'%.*s'%s",
      TCALC_TOKEN_PRINTF_VARARG(expr, globalTokenBuffer[i]),
      i == globalTokenBufferLen - 1 ? "" : ", "
    );
  }
  fputc('\n', stdout);

  return EXIT_SUCCESS;
}

