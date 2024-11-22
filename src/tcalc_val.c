#include "tcalc.h"

const char* tcalc_valtype_str(enum tcalc_valtype type) {
  switch (type) {
    case TCALC_VALTYPE_NUM: return "number";
    case TCALC_VALTYPE_BOOL: return "boolean";
  }
  // unreachable
  return "unknown";
}

void tcalc_val_fput(FILE* file, const struct tcalc_val val) {
  switch (val.type) {
    case TCALC_VALTYPE_BOOL:
      fprintf(file, "%s", TCALC_BOOLSTR(val.as.boolean));
      break;
    case TCALC_VALTYPE_NUM:
      fprintf(file, "%f", val.as.num);
      break;
  }
}

void tcalc_val_fputline(FILE* file, const struct tcalc_val val) {
  tcalc_val_fput(file, val);
  fputc('\n', file);
}
