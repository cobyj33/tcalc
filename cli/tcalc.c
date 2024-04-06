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

#define TCAPC_EXPRTREE_PRINT_MAX_DEPTH 20

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

struct eval_opts {
  int use_rads;
};

int tcalc_repl();
void tcalc_exprtree_print(tcalc_exprtree* node, size_t depth);
int tcalc_cli_expr_print_tree(const char* expr);
int tcalc_cli_eval(const char* expr, struct eval_opts eval_opts);
int tcalc_cli_eval_rpn(const char* expr, struct eval_opts eval_opts);
int tcalc_cli_rpn_tokenizer(const char* expr);
int tcalc_cli_infix_tokenizer(const char* expr);

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
    case TCALC_CLI_PRINT_EXPRTREE: return tcalc_cli_expr_print_tree(expression);
    case TCALC_CLI_PRINT_TOKENS: return tcalc_cli_infix_tokenizer(expression);
    case TCALC_CLI_PRINT_RPN_TOKENS: return tcalc_cli_rpn_tokenizer(expression);
    case TCALC_CLI_EVALUATE_RPN: return tcalc_cli_eval_rpn(expression, eval_opts);
    case TCALC_CLI_EVALUATE:
    default: return tcalc_cli_eval(expression, eval_opts);
  }
  return EXIT_SUCCESS;
}

int tcalc_cli_eval(const char* expr, struct eval_opts eval_opts) {
  double ans;
  tcalc_ctx* ctx = NULL;
  tcalc_err err = tcalc_ctx_alloc_default(&ctx);
  if (err) {
    fprintf(stderr, "[tcalc_cli_eval] TCalc error while allocating evaluation context: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
  }

  if (!eval_opts.use_rads) {
    err = tcalc_ctx_addtrigdeg(ctx);
    if (err) {
      fprintf(stderr, "[tcalc_cli_eval] TCalc error while switching to degree-trig functions: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
    }
  }

  err = tcalc_eval_wctx(expr, ctx, &ans);

  if (err) {
    fprintf(stderr, "TCalc error while evaluating expression: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
  }

  printf("%f\n", ans);
  return EXIT_SUCCESS;
}

int tcalc_cli_eval_rpn(const char* expr, struct eval_opts eval_opts) {
  double ans;
  tcalc_ctx* ctx = NULL;
  tcalc_err err = tcalc_ctx_alloc_default(&ctx);
  if (err) {
    fprintf(stderr, "[tcalc_cli_eval_rpn] TCalc error while allocating evaluation context: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
  }

  if (!eval_opts.use_rads) {
    err = tcalc_ctx_addtrigdeg(ctx);
    if (err) {
      fprintf(stderr, "[tcalc_cli_eval_rpn] TCalc error while switching to degree-trig functions: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
    }
  }

  err = tcalc_eval_rpn_wctx(expr, ctx, &ans);

  if (err) {
    fprintf(stderr, "[tcalc_cli_eval_rpn] TCalc error while evaluating expression: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
  }

  printf("%f\n", ans);
  return EXIT_SUCCESS;
}

const char* repl_entrance_text = "tcalc REPL begun: Enter \"quit\", \"exit\", or \"end\" to exit\n";

const char* repl_help = ""
"Enter \"quit\". \"exit\" or \"end\" to end the REPL session\n"
"Enter a mathematical expression to be evaluated\n"
"Ex: \"2 + 2 * (3/2)\", \"ln(e^3)\", \"arcsin(sin(pi/2))\", \"5 / (2pi+3)\"\n"
"\n"
"Special Keywords (case-sensitive):\n"
"\"quit\", \"exit\", or \"end\": Quit the REPL\n"
"\"variables\": Print all defined variables\n"
"\"functions\": Print all defined functions\n";

int tcalc_repl() {
  fputs(repl_entrance_text, stdout);
  char input_buffer[4096] = {'\0'};
  const char* quit_strings[3] = {"quit", "exit", "end"};
  
  tcalc_ctx* ctx = NULL;
  tcalc_err err = tcalc_ctx_alloc_default(&ctx);
  if (err) {
    fprintf(stderr, "Failed to allocate ctx for REPL... exiting: %s", tcalc_strerrcode(err));
    goto cleanup;
  }

  cleanup_on_err(err, tcalc_ctx_addvar(ctx, "ans", 0.0));

  while (!tcalc_str_list_has(input_buffer, quit_strings, ARRAY_SIZE(quit_strings))) {
    fputs("> ", stdout);
    char* input = fgets(input_buffer, 4096, stdin);
    if (input == NULL) {
      fprintf(stderr, "Error reading from stdin: Exiting\n");
      goto cleanup;
    }


    input[strcspn(input, "\r\n")] = '\0';
    if (tcalc_str_list_has(input, quit_strings,  ARRAY_SIZE(quit_strings))) break;

    if (strcmp(input, "help") == 0) {
      fputs(repl_help, stdout);
      continue;
    }
    else if (strcmp(input, "variables") == 0) {
      for (size_t i = 0; i < ctx->vars.len; i++) {
        printf("%s = %.5f\n", ctx->vars.arr[i]->id, ctx->vars.arr[i]->val);
      }
      continue;
    }
    else if (strcmp(input, "functions") == 0) {
      for (size_t i = 0; i < ctx->unfuncs.len; i++) {
        printf("%s%s", ctx->unfuncs.arr[i]->id, i == ctx->unfuncs.len - 1 ? "" : ", ");
      }

      for (size_t i = 0; i < ctx->binfuncs.len; i++) {
        printf("%s%s", ctx->binfuncs.arr[i]->id, i == ctx->binfuncs.len - 1 ? "" : ", ");
      }
      fputs("\n", stdout);
      continue;
    }

    double ans;
    tcalc_err err = tcalc_eval_wctx(input, ctx, &ans);
    if (err) {
      fprintf(stderr, "tcalc error: %s\n", tcalc_strerrcode(err));
    } else {
      printf("%f\n", ans);
    }

    fputs("\n", stdout);
    cleanup_on_err(err, tcalc_ctx_addvar(ctx, "ans", ans));
  }

  return EXIT_SUCCESS;

  cleanup:
    tcalc_ctx_free(ctx);
    return EXIT_FAILURE;
}

int tcalc_cli_expr_print_tree(const char* expr) {
  tcalc_ctx* ctx;
  tcalc_err err = tcalc_ctx_alloc_default(&ctx);
  if (err) {
    fprintf(stderr, "TCalc Error Occured: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
  }

  tcalc_exprtree* tree;
  err = tcalc_create_exprtree_infix(expr, ctx, &tree);
  tcalc_ctx_free(ctx);

  if (err) {
    fprintf(stderr, "TCalc Error Occured: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
  }

  tcalc_exprtree_print(tree, 0);
  tcalc_exprtree_free(tree);
  return EXIT_SUCCESS;
}

void tcalc_exprtree_print(tcalc_exprtree* node, size_t depth) {
  if (depth > TCAPC_EXPRTREE_PRINT_MAX_DEPTH) return;

  for (int i = 0; i < depth; i++)
    fputs("|___", stdout);
  printf("%s\n", node->token->val);

  for (size_t i = 0; i < node->nb_children; i++) {
    tcalc_exprtree_print(node->children[i], depth + 1);
  }
}

int tcalc_cli_rpn_tokenizer(const char* expr) {
  int exitcode = EXIT_SUCCESS;
  tcalc_ctx* ctx = NULL;
  tcalc_token** infix_tokens = NULL;
  tcalc_token** rpn_tokens = NULL;
  size_t nb_infix_tokens = 0;
  size_t nb_rpn_tokens = 0;

  tcalc_err err = tcalc_ctx_alloc_default(&ctx);
  if (err) {
    fprintf(stderr, "Error while allocating tcalc ctx: %s\n", tcalc_strerrcode(err));
    exitcode = EXIT_FAILURE; goto cleanup;
  }

  err = tcalc_tokenize_infix_ctx(expr, ctx, &infix_tokens, &nb_infix_tokens);
  if (err) {
    fprintf(stderr, "Error while tokenizing expr: %s\n ", tcalc_strerrcode(err));
    exitcode = EXIT_FAILURE; goto cleanup;
  }

  err = tcalc_infix_tokens_to_rpn_tokens(infix_tokens, nb_infix_tokens, ctx, &rpn_tokens, &nb_rpn_tokens);
  if (err) {
    fprintf(stderr, "Error while converting infix syntax to rpn syntax: %s\n", tcalc_strerrcode(err));
    exitcode = EXIT_FAILURE; goto cleanup;
  }

  for (size_t i = 0; i < nb_rpn_tokens; i++) {
    printf("{type: %s, value: '%s'}%s", tcalc_token_type_str(rpn_tokens[i]->type),  rpn_tokens[i]->val, i == nb_rpn_tokens - 1 ? "" : ", ");
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

  return exitcode;
}

int tcalc_cli_infix_tokenizer(const char* expr) {
  tcalc_token** tokens;
  size_t nb_tokens;
  
  tcalc_err err = tcalc_tokenize_infix(expr, &tokens, &nb_tokens);
  if (err) {
    printf("TCalc Error Occured: %s\n ", tcalc_strerrcode(err));
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

