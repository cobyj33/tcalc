#include "tcalc_mem.h"
#include <stdlib.h>

void tcalc_free_arr(void** arr, size_t size, void(*freecb)(void*)) {
  for (size_t i = 0; i < size; i++)
    freecb(arr[i]);
  free(arr);
}