
#include "tcalc_mem.h"
#include "tcalc_exit.h"
#include <stdlib.h>

void* tcalc_malloc(size_t size) {
  void* data = malloc(size);
  if (data == NULL) tcalc_die("[tcalc_malloc] bad malloc: %zu bytes", size);
  return data;
}

void* tcalc_calloc(size_t nmemb, size_t memsize) {
  void* data = calloc(nmemb, memsize);
  if (data == NULL) tcalc_die("[tcalc_calloc] bad calloc: %zu membs and %zu memsize", nmemb, memsize);
  return data;
}

void* tcalc_realloc(void* ptr, size_t newsize) {
  void* newptr = realloc(ptr, newsize);
  if (newptr == NULL) tcalc_die("[tcalc_realloc] bad realloc: %d bytes", newsize);
  return newptr;
}

void tcalc_free(void* ptr) { free(ptr); }