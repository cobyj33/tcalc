
#include "tcalc_error.h"
#include "tcalc_string.h"
#include <string.h>

char tcalc_error_string[TCALC_ERROR_MAX_SIZE] = {'\0'};

void tcalc_getfullerror(char* out) {
  strcpy(out, tcalc_error_string);
}

void tcalc_setfullerror(const char* error) {
  tcalc_strlcpy(tcalc_error_string, error, TCALC_ERROR_MAX_SIZE);
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
  }
}