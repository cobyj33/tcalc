#include "tcalc_cli_progs.h"
#include "tcalc_cli_common.h"

#include "tcalc_error.h"
#include "tcalc_context.h"
#include "tcalc_eval.h"
#include "tcalc_string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TCALC_REPL_INPUT_BUFFER_SIZE 4096

const char* repl_entrance_text = ""
"tcalc REPL begun:\n"
"  Enter \"quit\", \"exit\", or \"end\" to exit.\n"
"  Enter \"help\" to display a help text\n";

const char* repl_help = ""
"Enter \"quit\". \"exit\" or \"end\" to end the REPL session\n"
"\n"
"Enter a mathematical expression to be evaluated\n"
"  Ex: \"2 + 2 * (3/2)\", \"ln(e^3)\", \"arcsin(sin(pi/2))\", \"5 / (2pi+3)\"\n"
"\n"
"Special Keywords (case-sensitive):\n"
"  \"quit\", \"exit\", or \"end\": Quit the REPL\n"
"  \"variables\": Print all defined variables\n"
"  \"functions\": Print all defined functions\n";

int tcalc_repl() {
  fputs(repl_entrance_text, stdout);
  char input_buffer[TCALC_REPL_INPUT_BUFFER_SIZE] = {'\0'};
  const char* quit_strings[3] = {"quit", "exit", "end"};

  tcalc_ctx* ctx = NULL;
  tcalc_err err = tcalc_ctx_alloc_default(&ctx);
  TCALC_CLI_CLEANUP_ERR(err, "[%s] Failed to allocate ctx for REPL... exiting: %s", __func__, tcalc_strerrcode(err))

  err = tcalc_ctx_addvar(ctx, TCALC_STRLIT_PTR_LEN("ans"), TCALC_VAL_INIT_NUM(0.0));
  TCALC_CLI_CLEANUP_ERR(err, "[%s] Failed to set ans variable on tcalc ctx.. exiting: %s", __func__, tcalc_strerrcode(err))

  while (!tcalc_str_list_has(input_buffer, quit_strings, ARRAY_SIZE(quit_strings))) {
    fputs("> ", stdout);
    char* input = fgets(input_buffer, TCALC_REPL_INPUT_BUFFER_SIZE, stdin);
    if (input == NULL) {
      fprintf(stderr, "Error reading from stdin: Exiting\n");
      goto cleanup;
    }


    input[strcspn(input, "\r\n")] = '\0';
    if (tcalc_str_list_has(input, quit_strings,  ARRAY_SIZE(quit_strings)))
      break;

    if (strcmp(input, "help") == 0) {
      fputs(repl_help, stdout);
      continue;
    }
    else if (strcmp(input, "variables") == 0) {
      for (size_t i = 0; i < ctx->vars.len; i++) {
        const tcalc_vardef var = ctx->vars.arr[i];
        fputs(var.id, stdout);
        fputs(" = ", stdout);
        tcalc_val_fput(var.val, stdout);
        fputc('\n', stdout);
      }
      continue;
    }
    else if (strcmp(input, "functions") == 0) {
      for (size_t i = 0; i < ctx->unfuncs.len; i++) {
        printf("%s%s", ctx->unfuncs.arr[i].id, i == ctx->unfuncs.len - 1 ? "" : ", ");
      }

      for (size_t i = 0; i < ctx->binfuncs.len; i++) {
        printf("%s%s", ctx->binfuncs.arr[i].id, i == ctx->binfuncs.len - 1 ? "" : ", ");
      }
      fputs("\n", stdout);
      continue;
    }

    tcalc_val ans;
    tcalc_err err = tcalc_eval_wctx(input, ctx, &ans);
    if (err) {
      fprintf(stderr, "tcalc error: %s\n", tcalc_strerrcode(err));
      tcalc_errstk_printall();
    } else {
      tcalc_val_fput(ans, stdout);
      fputc('\n', stdout);
    }

    fputs("\n", stdout);
    cleanup_on_err(err, tcalc_ctx_addvar(ctx, TCALC_STRLIT_PTR_LEN("ans"), ans));
  }

  return EXIT_SUCCESS;

  cleanup:
    tcalc_ctx_free(ctx);
    return EXIT_FAILURE;
}
