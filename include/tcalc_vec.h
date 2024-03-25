#ifndef TCALC_VEC_H
#define TCALC_VEC_H

#include <stddef.h>
#include "tcalc_mem.h"

#define TCALC_VEC(type) tcalc_vec_ ## type

#define TCALC_DECLARE_VEC(type) typedef struct { \
    type* arr; \
    size_t len; \
    size_t cap; \
  } TCALC_VEC(type)

#define TCALC_VEC_PUSH(vec, item, err) TCALC_DARR_PUSH(vec.arr, vec.len, vec.cap, item, err)
#define TCALC_VEC_INSERT(vec, item, index, err) TCALC_DARR_INSERT(vec.arr, vec.len, vec.cap, item, index, err)
#define TCALC_VEC_GROW(vec, res_size, err) TCALC_DARR_GROW(vec.arr, res_size, vec.cap, err)

#endif