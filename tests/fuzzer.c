#include "tcalc_val.h"
#include "tcalc_eval.h"
#include <cstdint>
#include <cstddef>

// fuzz_target.cc
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  tcalc_val val;
  tcalc_err err = tcalc_eval((const char*)Data, );
  return err == TCALC_OK ? 0 : -1;  // Values other than 0 and -1 are reserved for future use.
}