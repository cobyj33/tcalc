#include "tcalc.h"
#include "tcalc_eval.h"
#include "tcalc_context.h"

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
"    --tokens-as-rpn: Print the reverse-polish notation tokens of the given expression\n";

int tcalc_repl();
void tcalc_exprtree_print(tcalc_exprtree* node, size_t depth);
int tcalc_cli_expr_print_tree(const char* expr);
int tcalc_cli_eval(const char* expr);
int tcalc_cli_eval_rpn(const char* expr);
int tcalc_cli_rpn_tokenizer(const char* expr);
int tcalc_cli_infix_tokenizer(const char* expr);

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

int main(int argc, char** argv) {
  enum tcalc_cli_action action = TCALC_CLI_EVALUATE;

  static struct option const longopts[] = {
    {"help", no_argument, NULL, 'h'},
    {"define", required_argument, NULL, arg_define},
    {"exprtree", no_argument, NULL, arg_exprtree},
    {"tokens", no_argument, NULL, arg_tokens},
    {"tokens-as-rpn", no_argument, NULL, arg_tokens_as_rpn},
    {"rpn", no_argument, NULL, arg_rpn},
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
    case TCALC_CLI_EVALUATE_RPN: return tcalc_cli_eval_rpn(expression);
    case TCALC_CLI_EVALUATE:
    default: return tcalc_cli_eval(expression);
  }
  return EXIT_SUCCESS;
}

int tcalc_cli_eval(const char* expr) {
  double ans;
  tcalc_err err = tcalc_eval(expr, &ans);

  if (err) {
    fprintf(stderr, "TCalc error while evaluating expression: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
  }

  printf("%f\n", ans);
  return EXIT_SUCCESS;
}

int tcalc_cli_eval_rpn(const char* expr) {
  double ans;
  tcalc_err err = tcalc_eval_rpn(expr, &ans);

  if (err) {
    fprintf(stderr, "TCalc error while evaluating expression: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
  }

  printf("%f\n", ans);
  return EXIT_SUCCESS;
}

int str_in_list(const char* input, const char** list, int count) {
  for (int i = 0; i < count; i++) {
    if (strcmp(input, list[i]) == 0) return 1;
  }
  return 0;
}

const char* repl_entrance_text = "tcalc REPL begun: Enter q to exit\n";

int tcalc_repl() {
  fputs(repl_entrance_text, stdout);
  char input_buffer[4096] = {'\0'};
  const char* quit_strings[4] = {"quit", "q", "exit", "end"};
  
  tcalc_ctx* context = NULL;
  tcalc_err err = tcalc_ctx_alloc_default(&context);
  if (err) {
    fprintf(stderr, "Failed to allocate context for REPL... exiting: %s", tcalc_strerrcode(err));
    goto cleanup;
  }

  while (!str_in_list(input_buffer, quit_strings, ARRAY_SIZE(quit_strings))) {
    fputs("> ", stdout);
    char* input = fgets(input_buffer, 4096, stdin);
    if (input == NULL) {
      fprintf(stderr, "Error reading from stdin: Exiting\n");
      goto cleanup;
    }

    input[strcspn(input, "\r\n")] = '\0';
    if (str_in_list(input, quit_strings,  ARRAY_SIZE(quit_strings))) break;

    double ans;
    tcalc_err err = tcalc_eval_wctx(input, context, &ans);
    if (err) {
      fprintf(stderr, "tcalc error: %s\n\n", tcalc_strerrcode(err));
    } else {
      printf("%f\n\n", ans);
    }

    tcalc_ctx_add_variable(context, "ans", ans);
  }

  return EXIT_SUCCESS;

  cleanup:
    tcalc_ctx_free(context);
    return EXIT_FAILURE;
}

int tcalc_cli_expr_print_tree(const char* expr) {
  tcalc_ctx* context;
  tcalc_err err = tcalc_ctx_alloc_default(&context);
  if (err) {
    fprintf(stderr, "TCalc Error Occured: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
  }

  tcalc_exprtree* tree;
  err = tcalc_create_exprtree_infix(expr, context, &tree);
  tcalc_ctx_free(context);

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
  printf("%s\n", node->token->value);

  for (size_t i = 0; i < node->nb_children; i++) {
    tcalc_exprtree_print(node->children[i], depth + 1);
  }
}

int tcalc_cli_rpn_tokenizer(const char* expr) {
  int exitcode = EXIT_SUCCESS;
  tcalc_ctx* context = NULL;
  tcalc_token** infix_tokens = NULL;
  tcalc_token** rpn_tokens = NULL;
  size_t nb_infix_tokens = 0;
  size_t nb_rpn_tokens = 0;

  tcalc_err err = tcalc_ctx_alloc_default(&context);
  if (err) {
    fprintf(stderr, "Error while allocating tcalc context: %s\n", tcalc_strerrcode(err));
    exitcode = EXIT_FAILURE; goto cleanup;
  }

  err = tcalc_tokenize_infix(expr, &infix_tokens, &nb_infix_tokens);
  if (err) {
    fprintf(stderr, "Error while tokenizing expr: %s\n ", tcalc_strerrcode(err));
    exitcode = EXIT_FAILURE; goto cleanup;
  }

  err = tcalc_infix_tokens_to_rpn_tokens(infix_tokens, nb_infix_tokens, context, &rpn_tokens, &nb_rpn_tokens);
  if (err) {
    fprintf(stderr, "Error while converting infix syntax to rpn syntax: %s\n", tcalc_strerrcode(err));
    exitcode = EXIT_FAILURE; goto cleanup;
  }

  for (size_t i = 0; i < nb_rpn_tokens; i++) {
    printf("{type: %s, value: '%s'}%s", tcalc_token_type_str(rpn_tokens[i]->type),  rpn_tokens[i]->value, i == nb_rpn_tokens - 1 ? "" : ", ");
  }
  fputs("\n\n", stdout);

  for (size_t i = 0; i < nb_rpn_tokens; i++) {
    printf("'%s'%s", rpn_tokens[i]->value, i == nb_rpn_tokens - 1 ? "" : ", ");
  }
  fputs("\n", stdout);

  cleanup:
  TCALC_ARR_FREE_F(infix_tokens, nb_infix_tokens, tcalc_token_free);
  TCALC_ARR_FREE_F(rpn_tokens, nb_rpn_tokens, tcalc_token_free);
  tcalc_ctx_free(context);

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
    printf("{type: %s, value: '%s'}, ", tcalc_token_type_str(tokens[i]->type),  tokens[i]->value);
  }
  printf("%s", "\n\n");

  for (size_t i = 0; i < nb_tokens; i++) {
    printf("'%s'%s", tokens[i]->value, i == nb_tokens - 1 ? "" : ", ");
  }
  printf("%c", '\n');

  TCALC_ARR_FREE_F(tokens, nb_tokens, tcalc_token_free);
  return EXIT_SUCCESS;
}

