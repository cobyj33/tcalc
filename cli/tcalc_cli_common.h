#ifndef TCALC_CLI_COMMON_H
#define TCALC_CLI_COMMON_H

#include "tcalc.h"

#include <stdio.h>
#include <stdio.h>

extern tcalc_token globalTokenBuffer[];
extern int32_t globalTokenBufferCapacity;
extern int32_t globalTokenBufferLen;

extern tcalc_exprtree globalTreeNodeBuffer[];
extern int32_t globalTreeNodeBufferCapacity;
extern int32_t globalTreeNodeBufferLen;
extern int32_t globalTreeNodeBufferRootIndex;

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
