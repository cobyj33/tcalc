#include "tcalc_darray.h"
#include "tcalc_error.h"
#include <malloc.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

const size_t TCALC_DARRAY_GROWTH_FACTOR = 2;

struct tcalc_darray_impl {
  void* data;
  size_t size;
  size_t capacity;
  size_t item_size;
};

void* tcalc_darray_vp_at(tcalc_darray* darray, size_t index);

tcalc_error_t tcalc_darray_expand(tcalc_darray* darray);

tcalc_darray* tcalc_darray_alloc(size_t item_size) {
  tcalc_darray* darray = (tcalc_darray*)malloc(sizeof(tcalc_darray));
  if (darray == NULL) {
    return NULL;
  }

  darray->data = NULL;
  darray->size = 0;
  darray->capacity = 0;
  darray->item_size = item_size;
  return darray;
}

void tcalc_darray_free(tcalc_darray* darray) {
  if (darray->data != NULL) {
    free(darray->data);
  }

  free(darray);
}

void tcalc_darray_free_cb(tcalc_darray* darray, void (*cb)(void*)) {
  if (darray->data != NULL) {
    for (size_t i = 0; i < darray->size; i++) {
      cb(tcalc_darray_vp_at(darray, i));
    }
    free(darray->data);
  }
  free(darray);
}

size_t tcalc_darray_size(tcalc_darray* darray) {
  return darray->size;
}

tcalc_error_t tcalc_darray_push(tcalc_darray* darray, void* data) {
  if (darray->size == darray->capacity) {
    tcalc_error_t err = tcalc_darray_expand(darray);
    if (err) return err;
  }

  memmove(tcalc_darray_vp_at(darray, darray->size), data, darray->item_size);
  darray->size++;
  return TCALC_OK;
}

tcalc_error_t tcalc_darray_peek(tcalc_darray* darray, void* out) {
  if (darray->size == 0) {
    return TCALC_INVALID_OP;
  }

  memmove(out, tcalc_darray_vp_at(darray, darray->size - 1), darray->item_size);
  return TCALC_OK;
}

tcalc_error_t tcalc_darray_pop(tcalc_darray* darray, void* out) {
  if (out != NULL) {
    tcalc_error_t err = tcalc_darray_peek(darray, out);
    if (err) return err;
  } 

  darray->size--;
  return TCALC_OK;
}

tcalc_error_t tcalc_darray_at(tcalc_darray* darray, void* out, size_t index) {
  if (index >= darray->size) {
    return TCALC_OUT_OF_BOUNDS;
  }

  memmove(out, tcalc_darray_vp_at(darray, index), darray->item_size);
  return TCALC_OK;
}

tcalc_error_t tcalc_darray_extract(tcalc_darray* darray, void** out) {
  *out = malloc(darray->item_size * darray->size);
  if (*out == NULL) {
    return TCALC_BAD_ALLOC;
  }

  memmove(*out, darray->data, darray->size * darray->item_size);
  return TCALC_OK;
}


tcalc_error_t tcalc_darray_expand(tcalc_darray* darray) {
  if (darray->capacity == 0) {
    size_t new_capacity = 1;
    darray->data = malloc(new_capacity * darray->item_size);
    if (darray->data == NULL) {
      return TCALC_BAD_ALLOC;
    }
    darray->capacity = new_capacity;
  } else {
    size_t new_capacity = darray->capacity * TCALC_DARRAY_GROWTH_FACTOR;
    void* new_data = realloc(darray->data, new_capacity * darray->item_size);
    if (new_data == NULL) {
      return TCALC_BAD_ALLOC;
    }

    darray->data = new_data;
    darray->capacity = new_capacity;
  }

  return TCALC_OK;
}

void* tcalc_darray_vp_at(tcalc_darray* darray, size_t index) {
  uint8_t* bytearr = (uint8_t*)darray->data;
  return (void*)(bytearr + (index * darray->item_size));
}