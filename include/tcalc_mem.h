#ifndef TCALC_MEM_H
#define TCALC_MEM_H

#include "tcalc_error.h"

#include <stddef.h>
#include <malloc.h>

/**
 * Free an allocated array. Also, free all data within the array with the
 * given callback.
 * 
 * You'll notice that alot of structs have a tcalc_struct_free and a
 * tcalc_struct_freev variant with a void pointer. It's literally just to
 * work with this bad boy.
 * (https://stackoverflow.com/a/559671)
*/
void tcalc_free_arr(void** arr, size_t size, void(*freecb)(void*));

/**
 * Inspired from the ALLOC_GROW API In git
 * 
 * ALLOC_GROW Documentation: https://github.com/git/git/blob/35f6318d44379452d8d33e880d8df0267b4a0cd0/Documentation/technical/api-allocation-growing.txt#L1-L20
 * Also in https://github.com/git/git/blob/master/git-compat-util.h
*/
tcalc_error_t tcalc_alloc_grow(void** arr, size_t item_size, size_t size, size_t* capacity);

#endif