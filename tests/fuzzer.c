#include "tcalc_val.h"
#include "tcalc_eval.h"
#include <stdint.h>
#include <stddef.h>

// fuzz_target.cc
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  tcalc_val val;
  // TODO: So, tcalc_eval should probably take an explicit string length then
  // huh?
  tcalc_err err = tcalc_eval((const char*)Data, );
  return err == TCALC_ERR_OK ? 0 : -1;  // Values other than 0 and -1 are reserved for future use.
}
