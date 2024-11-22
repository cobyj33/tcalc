#include "tcalc.h"
#include <stdlib.h>

void* tcalc_xmalloc(size_t size) {
  void* data = malloc(size);
  if (data == NULL) tcalc_die("[tcalc_malloc] bad malloc: %zu bytes", size);
  return data;
}

void* tcalc_xcalloc(size_t nmemb, size_t memsize) {
  void* data = calloc(nmemb, memsize);
  if (data == NULL) tcalc_die("[tcalc_calloc] bad calloc: %zu membs and %zu memsize", nmemb, memsize);
  return data;
}

void* tcalc_xrealloc(void* ptr, size_t newsize) {
  void* newptr = realloc(ptr, newsize);
  if (newptr == NULL) tcalc_die("[tcalc_realloc] bad realloc: %d bytes", newsize);
  return newptr;
}
