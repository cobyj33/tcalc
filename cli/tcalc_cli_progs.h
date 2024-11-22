#ifndef TCALC_CLI_PROGS_H
#define TCALC_CLI_PROGS_H

#include <stdbool.h>
#include <stdint.h>

struct eval_opts {
  bool use_rads;
};

int tcalc_repl();
int tcalc_cli_print_exprtree(const char* expr, int32_t exprLen);
int tcalc_cli_eval(const char* expr, int32_t exprLen, struct eval_opts eval_opts);
int tcalc_cli_infix_tokenizer(const char* expr, int32_t exprLen);

#endif
