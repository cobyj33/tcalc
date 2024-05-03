#include "tcalc_cli_progs.h"

#include "tcalc_cli_common.h"

#include "tcalc.h"
#include "tcalc_exprtree.h"
#include "tcalc_eval.h"
#include "tcalc_context.h"
#include "tcalc_tokens.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>


const char* TCALC_HELP_MESSAGE = "tcalc usage: tcalc [-h] expression \n"
"\n"
"Optional arguments:\n"
"    -h --help: Show this help message\n"
"    --define: define a variable\n"
"    --exprtree: Print the expression tree of the given expression\n"
"    --tokens: Print the tokens of the given expression\n"
"    --rpn: Evaluate the expression as reverse-polish notation syntax\n"
"    --tokens-as-rpn: Print the reverse-polish notation tokens of the given expression\n"
"    --degrees: Set trigonometric functions to be defined with degrees\n"
"    --radians: Set trigonometric functions to be defined with radians\n";

enum tcalc_cli_action {
  TCALC_CLI_PRINT_EXPRTREE,
  TCALC_CLI_PRINT_TOKENS,
  TCALC_CLI_PRINT_RPN_TOKENS,
  TCALC_CLI_EVALUATE_RPN,
  TCALC_CLI_EVALUATE
};


#define arg_define 43110
#define arg_exprtree 43111
#define arg_tokens 43112
#define arg_tokens_as_rpn 43113
#define arg_rpn 43114
#define arg_degrees 43115
#define arg_radians 43116


int main(int argc, char** argv) {
  enum tcalc_cli_action action = TCALC_CLI_EVALUATE;
  struct eval_opts eval_opts = { .use_rads = 1 };

  static struct option const longopts[] = {
    {"help", no_argument, NULL, 'h'},
    {"define", required_argument, NULL, arg_define},
    {"exprtree", no_argument, NULL, arg_exprtree},
    {"tokens", no_argument, NULL, arg_tokens},
    {"tokens-as-rpn", no_argument, NULL, arg_tokens_as_rpn},
    {"rpn", no_argument, NULL, arg_rpn},
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
      case arg_define: {
        fputs("Attempted to define an argument!", stdout);
        break;
      }
      case arg_exprtree: action = TCALC_CLI_PRINT_EXPRTREE; break;
      case arg_tokens: action = TCALC_CLI_PRINT_TOKENS; break;
      case arg_tokens_as_rpn: action = TCALC_CLI_PRINT_RPN_TOKENS; break;
      case arg_rpn: action = TCALC_CLI_EVALUATE_RPN; break;
      case arg_degrees: eval_opts.use_rads = 0; break;
      case arg_radians: eval_opts.use_rads = 1; break;
      default: {
        fputs(TCALC_HELP_MESSAGE, stderr);
        return EXIT_FAILURE;
      }
    }
  }

  if (optind >= argc) return tcalc_repl();
  char* expression = argv[optind];

  switch (action) {
    case TCALC_CLI_PRINT_EXPRTREE: return tcalc_cli_print_exprtree(expression);
    case TCALC_CLI_PRINT_TOKENS: return tcalc_cli_infix_tokenizer(expression);
    case TCALC_CLI_PRINT_RPN_TOKENS: return tcalc_cli_rpn_tokenizer(expression);
    case TCALC_CLI_EVALUATE_RPN: return tcalc_cli_eval_rpn(expression, eval_opts);
    case TCALC_CLI_EVALUATE:
    default: return tcalc_cli_eval(expression, eval_opts);
  }
  return EXIT_SUCCESS;
}
