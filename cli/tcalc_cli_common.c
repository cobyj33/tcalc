#include "tcalc_cli_common.h"

#include <stdio.h>
#include "tcalc_error.h"

void tcalc_errstk_printall() {
  while (tcalc_errstksize() > 0) {
    char err[512];
    tcalc_errstkpeek(err, 512);
    fprintf(stderr, "%s\n", err);
    tcalc_errstkpop();
  }
}