#ifndef TCALC_CLI_COMMON_H
#define TCALC_CLI_COMMON_H

#include <stdio.h>
#include "tcalc.h"

#define TCALC_CLI_CHECK_ERR(err, ...) \
  if (err) { \
    fprintf(stderr, __VA_ARGS__); \
    tcalc_errstk_fdump(stderr); \
    return EXIT_FAILURE; \
  }

#define TCALC_CLI_CLEANUP_ERR(err, ...) \
  if (err) { \
    fprintf(stderr, __VA_ARGS__); \
    tcalc_errstk_fdump(stderr); \
    goto cleanup; \
  }

#endif
