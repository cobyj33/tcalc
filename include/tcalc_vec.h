#ifndef TCALC_VEC_H
#define TCALC_VEC_H

#include <stddef.h>
#include "tcalc_mem.h"

#define TCALC_VEC(type) struct { \
    type* arr; \
    size_t len; \
    size_t cap; \
  }

#define TCALC_VEC_INIT { .arr = NULL, .len = 0, .cap = 0 }
#define TCALC_VEC_FREE(vec) do { \
  free((vec).arr); \
  (vec).arr = NULL; \
  (vec).len = 0; \
  (vec).cap = 0; \
  } while (0)


#define TCALC_VEC_PUSH(vec, item, err) TCALC_DARR_PUSH(vec.arr, vec.len, vec.cap, item, err)
#define TCALC_VEC_INSERT(vec, item, index, err) TCALC_DARR_INSERT(vec.arr, vec.len, vec.cap, item, index, err)
#define TCALC_VEC_GROW(vec, res_size, err) TCALC_DARR_GROW(vec.arr, res_size, vec.cap, err)
#define TCALC_VEC_FOREACH(vec, iname) for (size_t iname = 0; iname < vec.len; iname++)

#define TCALC_VEC_FREE_F(vec, freefn) do { \
  TCALC_ARR_FREE_F(vec.arr, vec.len, freefn); \
  TCALC_VEC_FREE(vec); \
} while (0)

#define TCALC_VEC_FREE_FV(vec, freefnv) do { \
  TCALC_ARR_FREE_F(vec.arr, vec.len, freefnv); \
  TCALC_VEC_FREE(vec); \
} while (0)

#endif
