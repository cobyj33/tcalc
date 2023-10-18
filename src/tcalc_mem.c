#include "tcalc_mem.h"
#include "tcalc_error.h"
#include <stdlib.h>

void tcalc_free_arr(void** arr, size_t size, void(*freecb)(void*)) {
  for (size_t i = 0; i < size; i++)
    freecb(arr[i]);
  free(arr);
}

#define alloc_nr(x) (((x)+16)*3/2)

tcalc_error_t tcalc_alloc_grow(void** arr, size_t item_size, size_t size, size_t* capacity) {
  if (*capacity == 0) {
    free(*arr);
    size_t new_capacity = 1;
    *arr = malloc(item_size * new_capacity);
    if (*arr == NULL) {
      return TCALC_BAD_ALLOC;
    }
    *capacity = new_capacity;
  } else if (size > *capacity) {
    size_t new_capacity = *capacity;
    if (alloc_nr(*capacity) < size) {
      new_capacity = size;
    } else {
      new_capacity = alloc_nr(*capacity);
    }

    void* realloced = realloc(*arr, item_size * new_capacity);
    if (realloced == NULL)
      return TCALC_BAD_ALLOC;
    *arr = realloced;
    *capacity = new_capacity;
  }

  return TCALC_OK;
}

#undef alloc_nr