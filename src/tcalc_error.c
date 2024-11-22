#include "tcalc.h"

#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void tcalc_die(const char *err, ...) {
	char msg[4096];
	va_list params;
	va_start(params, err);
	vsnprintf(msg, sizeof(msg), err, params);
	fprintf(stderr, "%s\n", msg);
	va_end(params);
	exit(1);
}

#define TCALC_ERROR_MAX_SIZE 256
#define TCALC_ERRSTK_MAX_SIZE 16

int errstksize = 0;
char errstk[TCALC_ERRSTK_MAX_SIZE][TCALC_ERROR_MAX_SIZE] = {'\0'};

const char* tcalc_strerrcode(tcalc_err err) {
  switch (err) {
    case TCALC_ERR_OK: return "ok";
    case TCALC_ERR_OUT_OF_BOUNDS: return "out of bounds";
    case TCALC_ERR_NOMEM: return "no available memory";
    case TCALC_ERR_INVALID_ARG: return "invalid argument";
    case TCALC_ERR_INVALID_OP: return "invalid opertion";
    case TCALC_ERR_OVERFLOW: return "overflow";
    case TCALC_ERR_UNDERFLOW: return "underflow";
    case TCALC_ERR_STOP_ITER: return "stop iteration";
    case TCALC_ERR_NOT_FOUND: return "not found";
    case TCALC_ERR_DIV_BY_ZERO: return "division by zero";
    case TCALC_ERR_NOT_IN_DOMAIN: return "not in domain";
    case TCALC_ERR_UNBAL_GRPSYMS: return "unbalanced grouping symbols";
    case TCALC_ERR_UNKNOWN_TOKEN: return "unknown token";
    case TCALC_ERR_WRONG_ARITY: return "wrong arity";
    case TCALC_ERR_UNCLOSED_FUNC: return "unclosed function";
    case TCALC_ERR_UNCALLED_FUNC: return "uncalled function";
    case TCALC_ERR_MALFORMED_BINEXP: return "malformed binary expression";
    case TCALC_ERR_MALFORMED_UNEXP: return "malformed unary expression";
    case TCALC_ERR_MALFORMED_INPUT: return "malformed input";
    case TCALC_ERR_MALFORMED_FUNC: return "malformed function";
    case TCALC_ERR_FUNC_TOO_MANY_ARGS: return "too many arguments";
    case TCALC_ERR_BAD_CAST: return "bad cast";
    case TCALC_ERR_UNPROCESSED_INPUT: return "unprocessed input";
    case TCALC_ERR_UNIMPLEMENTED: return "unimplemented";
    case TCALC_ERR_UNKNOWN_ID: return "unknown identifier";
    case TCALC_ERR_UNKNOWN: return "unknown";
  }

  return "unknown tcalc error";
}

int tcalc_errstksize() {
  return errstksize;
}

void tcalc_errstkclear() {
  errstksize = 0;
}

bool tcalc_errstkadd(const char* funcname, const char* errstr) {
  if (errstksize >= TCALC_ERRSTK_MAX_SIZE) return false;
  int res = snprintf(errstk[errstksize], TCALC_ERROR_MAX_SIZE, "[%s] %s", funcname, errstr);

  if (res < 0) {
    errstk[errstksize][0] = '\0';
    return false;
  }

  errstksize++;
  return true;
}

bool tcalc_errstkaddf(const char* funcname, const char* format, ...) {
  if (errstksize >= TCALC_ERRSTK_MAX_SIZE) return false;
  char err[TCALC_ERROR_MAX_SIZE] = {'\0'};

  va_list args;
  va_start(args, format);
  int success = vsnprintf(err, TCALC_ERROR_MAX_SIZE, format, args);
  va_end(args);

  if (!success) return false;
  return tcalc_errstkadd(funcname, err);
}

int32_t tcalc_errstkpeek(char* out, int32_t dsize) {
  if (errstksize == 0) return 0;
  return tcalc_strcpy_lblb_ntdst(
    out, dsize,
    errstk[errstksize - 1], (int32_t)strlen(errstk[errstksize - 1])
  );
}

int tcalc_errstkpop() {
  if (errstksize == 0) return 0;
  errstksize--;
  return errstksize;
}

void tcalc_errstk_fdump(FILE* file) {
  while (tcalc_errstksize() > 0) {
    char err[512];
    tcalc_errstkpeek(err, 512);
    fprintf(file, "%s\n", err);
    tcalc_errstkpop();
  }
}
