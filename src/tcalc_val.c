#include "tcalc.h"

const char* tcalc_valtype_str(enum tcalc_valtype type) {
  switch (type) {
    case TCALC_VALTYPE_NUM: return "number";
    case TCALC_VALTYPE_BOOL: return "boolean";
  }
  // unreachable
  return "unknown";
}
