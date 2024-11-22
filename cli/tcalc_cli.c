#include "tcalc_cli_progs.h"
#include "tcalc_cli_common.h"

#include "tcalc.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>

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
  TCALC_CLI_PRINT_EXPRTREE,
  TCALC_CLI_PRINT_TOKENS,
  TCALC_CLI_EVALUATE
};


#define arg_define 43110
#define arg_exprtree 43111
#define arg_tokens 43112
#define arg_degrees 43115
#define arg_radians 43116


int main(int argc, char** argv) {
  enum tcalc_cli_action action = TCALC_CLI_EVALUATE;
  struct eval_opts eval_opts = { .use_rads = true };

  static struct option const longopts[] = {
    {"help", no_argument, NULL, 'h'},
    {"exprtree", no_argument, NULL, arg_exprtree},
    {"tokens", no_argument, NULL, arg_tokens},
    {"degrees", no_argument, NULL, arg_degrees},
    {"radians", no_argument, NULL, arg_radians},
    {NULL, 0, NULL, 0},
  };

  int opt, long_index;
  while ((opt = getopt_long(argc, argv, "h", longopts, &long_index)) != -1) {
    switch (opt) {
      case 'h': {
        fputs(TCALC_HELP_MESSAGE, stdout);
        return EXIT_SUCCESS;
      }
      case arg_exprtree: action = TCALC_CLI_PRINT_EXPRTREE; break;
      case arg_tokens: action = TCALC_CLI_PRINT_TOKENS; break;
      case arg_degrees: eval_opts.use_rads = false; break;
      case arg_radians: eval_opts.use_rads = true; break;
      default: {
        fputs(TCALC_HELP_MESSAGE, stderr);
        return EXIT_FAILURE;
      }
    }
  }

  if (optind >= argc) return tcalc_repl();
  char* expression = argv[optind];
  size_t expressionLenSizeT = strlen(expression);
  if (expressionLenSizeT > INT32_MAX)
  {
    fprintf(
      stderr,
      "Attempted to parse a string far too large (Larger than %" PRId32 " bytes)",
      INT32_MAX
    );
    return EXIT_FAILURE;
  }

  const int32_t expressionLen = (int32_t)expressionLenSizeT;

  switch (action) {
    case TCALC_CLI_PRINT_EXPRTREE:
      return tcalc_cli_print_exprtree(expression, expressionLen);
    case TCALC_CLI_PRINT_TOKENS:
      return tcalc_cli_infix_tokenizer(expression, expressionLen);
    case TCALC_CLI_EVALUATE:
    default:
      return tcalc_cli_eval(expression, expressionLen, eval_opts);
  }
  return EXIT_SUCCESS;
}
