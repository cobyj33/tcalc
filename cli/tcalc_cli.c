#include "tcalc_cli_progs.h"
#include "tcalc_cli_common.h"

#include "tcalc.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>

#include "ccmdlpar.h"

tcalc_token globalTokenBuffer[TCALC_KIBI(512)];
int32_t globalTokenBufferCapacity = (int32_t)TCALC_ARRAY_SIZE(globalTokenBuffer);
int32_t globalTokenBufferLen = 0;

tcalc_exprtree globalTreeNodeBuffer[TCALC_KIBI(512)];
int32_t globalTreeNodeBufferCapacity = (int32_t)TCALC_ARRAY_SIZE(globalTreeNodeBuffer);
int32_t globalTreeNodeBufferLen = 0;
int32_t globalTreeNodeBufferRootIndex = -1;


const char* TCALC_HELP_MESSAGE = "tcalc usage: tcalc [-h] expression \n"
"\n"
"Optional arguments:\n"
"    -h --help: Show this help message\n"
"    --exprtree: Print the expression tree of the given expression\n"
"    --tokens: Print the tokens of the given expression\n"
"    --degrees: Set trigonometric functions to be defined with degrees\n"
"    --radians: Set trigonometric functions to be defined with radians\n";

enum tcalc_cli_action {
  TCALC_CLI_ACTION_EVALUATE = (1 << 0),
  TCALC_CLI_ACTION_PRINT_EXPRTREE = (1 << 1),
  TCALC_CLI_ACTION_PRINT_TOKENS = (1 << 2),
};

int main(int argc, char** argv) {
  enum tcalc_cli_action action = TCALC_CLI_ACTION_EVALUATE;
  struct eval_opts eval_opts = { .use_rads = true };

  const char* soptsWArgs = "";
  const char* loptsWArgs[] = { NULL };
  struct ccmdlpar_state cmdlState = CCMDLPAR_STATE_INIT;
  struct ccmdlpar_state cmdlStateLast = CCMDLPAR_STATE_INIT;
  struct ccmdlpar_parse_info cmdlParseInfo = { 0 };
  enum ccmdlpar_status cmdlStatus = CCMDLPAR_STATUS_OK;

  bool cmdlAnyError = false;
  while ((cmdlStatus = ccmdlpar(argc, (const char* const*)argv, &cmdlState, soptsWArgs, loptsWArgs, &cmdlParseInfo)) != CCMDLPAR_STATUS_FINISHED)
  {
    if (cmdlStatus != CCMDLPAR_STATUS_OK)
    {
      fprintf(
        stderr, "[tcalc::%s] Error while parsing argument at offset %d/%d '%s': %s\n",
        __func__,
        cmdlStateLast.optind, argc, argv[cmdlStateLast.optind],
        ccmdlpar_status_str(cmdlStatus)
      );
      cmdlAnyError = true;
    }
    else if (cmdlParseInfo.argType == CCMDLPAR_ARG_TYPE_SHORT_OPTION || cmdlParseInfo.argType == CCMDLPAR_ARG_TYPE_LONG_OPTION)
    {
      if (ccmdlpar_found_sopt(cmdlParseInfo, 'h') || ccmdlpar_found_lopt(cmdlParseInfo, "help"))
      {
        fputs(TCALC_HELP_MESSAGE, stdout);
        return EXIT_SUCCESS;
      }
      else if (ccmdlpar_found_lopt(cmdlParseInfo, "exprtree"))
      {
        action |= TCALC_CLI_ACTION_PRINT_EXPRTREE;
      }
      else if (ccmdlpar_found_lopt(cmdlParseInfo, "tokens"))
      {
        action |= TCALC_CLI_ACTION_PRINT_TOKENS;
      }
      else if (ccmdlpar_found_lopt(cmdlParseInfo, "eval"))
      {
        action |= TCALC_CLI_ACTION_EVALUATE;
      }
      else if (ccmdlpar_found_lopt(cmdlParseInfo, "no-eval"))
      {
        action &= ~TCALC_CLI_ACTION_EVALUATE;
      }
      else if (ccmdlpar_found_lopt(cmdlParseInfo, "degrees"))
      {
        eval_opts.use_rads = false;
      }
      else if (ccmdlpar_found_lopt(cmdlParseInfo, "radians"))
      {
        eval_opts.use_rads = true;
      }
      else
      {
        fprintf(
          stderr, "[tcalc::%s] Unknown Option Argument: '%s%.*s'\n",
          __func__,
          ccmdlpar_arg_type_prefix(cmdlParseInfo.argType),
          cmdlParseInfo.argLen, cmdlParseInfo.arg
        );
        cmdlAnyError = true;
      }
    }

    cmdlStateLast = cmdlState;
  }


  if (cmdlAnyError)
    return EXIT_FAILURE;

  cmdlState = CCMDLPAR_STATE_INIT;
  int exitCode = EXIT_SUCCESS;
  bool cmdlHasNonOption = false;
  while ((cmdlStatus = ccmdlpar(argc, (const char* const*)argv, &cmdlState, soptsWArgs, loptsWArgs, &cmdlParseInfo)) != CCMDLPAR_STATUS_FINISHED)
  {
    assert(cmdlStatus == CCMDLPAR_STATUS_OK);
    if (cmdlParseInfo.argType == CCMDLPAR_ARG_TYPE_NON_OPTION)
    {
      cmdlHasNonOption = true;
      const char* expr = cmdlParseInfo.arg;
      size_t exprLenSizeT = strlen(expr);
      if (exprLenSizeT > INT32_MAX)
      {
        fprintf(
          stderr,
          "Attempted to parse a string far too large (Larger than %" PRId32 " bytes)",
          INT32_MAX
        );
        exitCode = EXIT_FAILURE;
        continue;
      }

      const int32_t exprLenI32 = (int32_t)exprLenSizeT;

      if (action & TCALC_CLI_ACTION_PRINT_TOKENS)
      {
        const int tokenExitCode = tcalc_cli_infix_tokenizer(expr, exprLenI32);
        if (exitCode == EXIT_SUCCESS) exitCode = tokenExitCode;
      }
      if (action & TCALC_CLI_ACTION_PRINT_EXPRTREE)
      {
        const int exprExitCode = tcalc_cli_print_exprtree(expr, exprLenI32);
        if (exitCode == EXIT_SUCCESS) exitCode = exprExitCode;
      }
      if (action & TCALC_CLI_ACTION_EVALUATE)
      {
        const int evalExitCode = tcalc_cli_eval(expr, exprLenI32, eval_opts);
        if (exitCode == EXIT_SUCCESS) exitCode = evalExitCode;
      }
    }
  }

  if (!cmdlHasNonOption)
  {
    exitCode = tcalc_repl();
  }

  return exitCode;
}
