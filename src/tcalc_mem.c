#include "tcalc_mem.h"
#include "tcalc_error.h"
#include <stdlib.h>

void tcalc_free_arr(void** arr, size_t size, void(*freecb)(void*)) {
  for (size_t i = 0; i < size; i++)
    freecb(arr[i]);
  free(arr);
}

#define alloc_nr(x) (((x)+16)*3/2)

tcalc_error_t tcalc_alloc_grow(void** arr, size_t item_size, size_t size, size_t* alloc) {
  if (*alloc == 0) {
    free(*arr);
    size_t new_alloc_size = 1;
    *arr = malloc(item_size * new_alloc_size);
    if (*arr == NULL) {
      return TCALC_BAD_ALLOC;
    }
    *alloc = new_alloc_size;
  } else if (size > *alloc) {
    size_t new_alloc_size = *alloc;
    if (alloc_nr(*alloc) < size) {
      new_alloc_size = size;
    } else {
      new_alloc_size = alloc_nr(*alloc);
    }

    void* realloced = realloc(*arr, item_size * new_alloc_size);
    if (realloced == NULL)
      return TCALC_BAD_ALLOC;
    *arr = realloced;
    *alloc = new_alloc_size
  }

  return TCALC_OK;
}

#undef alloc_nr