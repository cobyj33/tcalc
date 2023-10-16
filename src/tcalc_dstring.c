#include "tcalc_string.h"
#include "tcalc_dstring.h"
#include <stdlib.h>
#include <string.h>

const size_t TCALC_DSTRING_GROWTH_FACTOR = 2;
const size_t TCALC_DSTRING_INITIAL_SIZE = 1;

struct tcalc_dstring_impl {
  char* buffer;
  size_t size;
  size_t capacity;
};

tcalc_error_t tcalc_dstring_expand(tcalc_dstring*);
tcalc_error_t tcalc_dstring_expand_to(tcalc_dstring*, size_t);

tcalc_dstring* tcalc_dstring_alloc() {
  tcalc_dstring* str = (tcalc_dstring*)malloc(sizeof(tcalc_dstring));
  if (str == NULL) {
    return NULL;
  }
  str->buffer = (char*)malloc(sizeof(char) * TCALC_DSTRING_INITIAL_SIZE);
  if (str->buffer == NULL) {
    free(str);
    return NULL;
  }

  str->buffer[0] = '\0';
  str->size = 0;
  str->capacity = TCALC_DSTRING_INITIAL_SIZE;
  return str;
}

void tcalc_dstring_clear(tcalc_dstring* str) {
  str->buffer[0] = '\0';
  str->size = 0;  
}

tcalc_error_t tcalc_dstring_set_csubstr(tcalc_dstring* str, const char* src, size_t sub_str_start, size_t sub_str_end) {
  size_t sub_str_len = sub_str_end - sub_str_start;
  tcalc_error_t err = tcalc_dstring_expand_to(str, sub_str_len + 1);
  if (err) return err;

  tcalc_strlcpy(str->buffer, src + sub_str_start, sub_str_len + 1);
  str->size = sub_str_len;
  return TCALC_OK;
}

tcalc_error_t tcalc_dstring_strlcpy_get(tcalc_dstring* str, char* dest, size_t dsize) {
  if (str->size < dsize) {
    return TCALC_INVALID_ARG;
  }

  tcalc_strlcpy(dest, str->buffer, dsize);
  return TCALC_OK;
}

tcalc_error_t tcalc_dstring_cstrdup(tcalc_dstring* str, char** out) {
  return tcalc_strdup(str->buffer, out);
}

int tcalc_dstring_strcmp(tcalc_dstring* str, const char* other) {
  return strcmp(str->buffer, other);
}

tcalc_error_t tcalc_dstring_set_cstr(tcalc_dstring* str, const char* src) {
  size_t len = strlen(src);
  tcalc_error_t err = tcalc_dstring_expand_to(str, len + 1);
  if (err) return err;

  strcpy(str->buffer, src);
  str->size = len;
  return TCALC_OK;
}

size_t tcalc_dstring_size(tcalc_dstring* str) {
  return str->size;
}

void tcalc_dstring_free(tcalc_dstring* str) {
  free(str->buffer);
  free(str);
}

tcalc_error_t tcalc_dstring_append_cstr(tcalc_dstring* str, char* src) {
  size_t new_size = str->size + strlen(src);  
  tcalc_error_t err = tcalc_dstring_expand_to(str, new_size + 1);
  if (err) return err;

  strcat(str->buffer, src);
  str->size = new_size;
  return TCALC_OK;
}

tcalc_error_t tcalc_dstring_expand(tcalc_dstring* str) {
  if ( ((size_t)-1) / TCALC_DSTRING_GROWTH_FACTOR <= str->capacity) {
    return TCALC_OVERFLOW;
  }

  size_t new_capacity = str->capacity * TCALC_DSTRING_GROWTH_FACTOR;
    
  char* new_buffer = (char*)realloc(str->buffer, new_capacity * sizeof(char));
  if (new_buffer == NULL) {
    return TCALC_BAD_ALLOC;
  }

  str->buffer = new_buffer;
  str->capacity = new_capacity;
  return TCALC_OK;
}

tcalc_error_t tcalc_dstring_expand_to(tcalc_dstring* str, size_t minimum) {
  size_t new_capacity = str->capacity;
  while (new_capacity < minimum) {
    if ( ((size_t)-1) / TCALC_DSTRING_GROWTH_FACTOR <= new_capacity) {
      return TCALC_OVERFLOW;
    }

    new_capacity *= TCALC_DSTRING_GROWTH_FACTOR;
  }
  
  char* new_buffer = (char*)realloc(str->buffer, new_capacity * sizeof(char));
  if (new_buffer == NULL) {
    return TCALC_BAD_ALLOC;
  }

  str->buffer = new_buffer;
  str->capacity = new_capacity;
  return TCALC_OK;
}

int tcalc_dstring_isint(tcalc_dstring* str) {
  return tcalc_strisint(str->buffer);
}

int tcalc_dstring_isdouble(tcalc_dstring* str) {
  return tcalc_strisdouble(str->buffer);
}

tcalc_error_t tcalc_dstring_getint(tcalc_dstring* str, int* out) {
  return tcalc_strtoint(str->buffer, out);
}

tcalc_error_t tcalc_dstring_getdouble(tcalc_dstring* str, double* out) {
  return tcalc_strtodouble(str->buffer, out);
}