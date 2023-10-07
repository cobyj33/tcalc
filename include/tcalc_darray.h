#ifndef TCALC_DARRAY_H
#define TCALC_DARRAY_H

#include <stddef.h>
#include "tcalc_error.h"

typedef struct tcalc_darray_impl tcalc_darray;

tcalc_darray* tcalc_darray_alloc(size_t item_size);
void tcalc_darray_free(tcalc_darray*);
void tcalc_darray_free_cb(tcalc_darray*, void (*)(void*));

size_t tcalc_darray_size(tcalc_darray*);
tcalc_error_t tcalc_darray_push(tcalc_darray*, void*);

tcalc_error_t tcalc_darray_at(tcalc_darray*, void*, size_t);
tcalc_error_t tcalc_darray_pop(tcalc_darray*, void*);
tcalc_error_t tcalc_darray_peek(tcalc_darray* darray, void* out);

tcalc_error_t tcalc_darray_extract(tcalc_darray*, void**);

#endif