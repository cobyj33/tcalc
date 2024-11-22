#include "tcalc_cli_common.h"

#include <stdio.h>
#include "tcalc.h"

void tcalc_errstk_printall() {
  while (tcalc_errstksize() > 0) {
    char err[512];
    tcalc_errstkpeek(err, 512);
    fprintf(stderr, "%s\n", err);
    tcalc_errstkpop();
  }
}

void tcalc_val_fput(const struct tcalc_val val, FILE* file) {
  switch (val.type) {
    case TCALC_VALTYPE_BOOL:
      printf("%s", val.as.boolean ? "true" : "false");
      break;
    case TCALC_VALTYPE_NUM:
      printf("%f", val.as.num);
      break;
  }
}
