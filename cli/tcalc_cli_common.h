#ifndef TCALC_CLI_COMMON_H
#define TCALC_CLI_COMMON_H

void tcalc_errstk_printall();

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