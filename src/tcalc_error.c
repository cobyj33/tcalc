
#include "tcalc_error.h"
#include "tcalc_string.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

char tcalc_error_string[TCALC_ERROR_MAX_SIZE] = {'\0'};

void tcalc_getfullerror(char* out) {
  strcpy(out, tcalc_error_string);
}

void tcalc_setfullerror(const char* error) {
  tcalc_strlcpy(tcalc_error_string, error, TCALC_ERROR_MAX_SIZE);
}

void tcalc_setfullerrorf(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vsnprintf(tcalc_error_string, TCALC_ERROR_MAX_SIZE, format, args);
  tcalc_error_string[TCALC_ERROR_MAX_SIZE - 1] = '\0';
  va_end(args);
}

const char* tcalc_strerrcode(tcalc_error_t err) {
  switch (err) {
    case TCALC_OK: return "ok";
    case TCALC_OUT_OF_BOUNDS: return "out of bounds";
    case TCALC_BAD_ALLOC: return "bad alloc";
    case TCALC_INVALID_ARG: return "invalid argument";
    case TCALC_INVALID_OP: return "invalid opertion";
    case TCALC_OVERFLOW: return "overflow";
    case TCALC_UNDERFLOW: return "underflow";
    case TCALC_STOP_ITER: return "stop iteration";
    case TCALC_NOT_FOUND: return "not found";
    case TCALC_DIVISION_BY_ZERO: return "division by zero";
    case TCALC_NOT_IN_DOMAIN: return "not in domain";
    case TCALC_UNBALANCED_GROUPING_SYMBOLS: return "unbalanced grouping symbols";
    case TCALC_UNKNOWN_TOKEN: return "unknown token";
    case TCALC_UNIMPLEMENTED: return "unimplemented";
    case TCALC_UNKNOWN_IDENTIFIER: return "unknown identifier";
    case TCALC_UNKNOWN: return "unknown";
  }

  return "unknown tcalc error";
}