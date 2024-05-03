#ifndef TCALC_CLI_PROGS_H
#define TCALC_CLI_PROGS_H

struct eval_opts {
  int use_rads;
};

int tcalc_repl();
int tcalc_cli_print_exprtree(const char* expr);
int tcalc_cli_eval(const char* expr, struct eval_opts eval_opts);
int tcalc_cli_eval_rpn(const char* expr, struct eval_opts eval_opts);
int tcalc_cli_rpn_tokenizer(const char* expr);
int tcalc_cli_infix_tokenizer(const char* expr);

#endif