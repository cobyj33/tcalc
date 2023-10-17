#ifndef TCALC_DARRAY_H
#define TCALC_DARRAY_H

#include <stddef.h>
#include "tcalc_error.h"

typedef struct tcalc_darray_impl tcalc_darray;

/**
 * Allocates a darray
 * 
 * The caller is responsible for only pushing data of the determined type
 * into the darray. I suggest placing a comment of the "c-like" tyoe that
 * the array would have next to the alloc call
*/
tcalc_darray* tcalc_darray_alloc(size_t item_size);

/**
 * 
*/
void tcalc_darray_free(tcalc_darray*);

/**
 * This should only be called if you have ALL pointers that point at memory
 * allocated spaces in your darray. 
*/
void tcalc_darray_free_cb(tcalc_darray*, void (*)(void*));

size_t tcalc_darray_size(tcalc_darray*);

/**
 * Copies data located at the address data_adr into the darray
 * 
 * @param darray - the darray to push 
 * @param data - the data address to **copy** the data located at the address into the darray 
 * 
 * by copy, I mean the darray will copy the data located AT the pointer, not the
 * pointer itself. This means stack allocated data like integers can be copied into
 * the darray and the data will persist in the darray once the stack allocated data is inevitably freed,
 * since the data of the integer has been copied into the darray's buffer. 
*/
tcalc_error_t tcalc_darray_push(tcalc_darray*, void*);

tcalc_error_t tcalc_darray_at(tcalc_darray*, void*, size_t);


tcalc_error_t tcalc_darray_pop(tcalc_darray*, void*);


tcalc_error_t tcalc_darray_peek(tcalc_darray* darray, void* out);


/**
 * Copies the data currently
*/
tcalc_error_t tcalc_darray_extract(tcalc_darray*, void**);

#endif