#ifndef TCALC_CLI_COMMON_H
#define TCALC_CLI_COMMON_H

#include <stdio.h>

struct tcalc_val;

void tcalc_errstk_printall();

// Note that tcalc_val_fput does NOT print a trailing newline. This
// is to allow using tcalc_val_fput to put multiple values on the same
// line in 'file'.
void tcalc_val_fput(const struct tcalc_val val, FILE* file);

#define TCALC_CLI_CHECK_ERR(err, ...) \
  if (err) { \
    fprintf(stderr, __VA_ARGS__); \
    tcalc_errstk_printall(); \
    return EXIT_FAILURE; \
  }

#define TCALC_CLI_CLEANUP_ERR(err, ...) \
  if (err) { \
    fprintf(stderr, __VA_ARGS__); \
    tcalc_errstk_printall(); \
    goto cleanup; \
  }

#endif
