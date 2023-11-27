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

#define alloc_nr(x) (((x)+16)*3/2)

/**
 * 
 * 
 * arr: The pointer to the array to perform possible growth on
 * size: The size to which the array should fit into
 * capacity: The current capacity of the array
 * err: a tcalc_error_t variable that will be set upon any errors
 * 
 * the capacity variable will be updated to the current array's capacity if
 * reallocated, or will stay the same under no reallocation
 * 
 * the arr variable will be altered, if reallocated, to point to the reallocated
 * data
*/
#define TCALC_DARR_GROW(arr, size, capacity, err) do { \
    if (capacity == 0) { \
      arr = malloc(sizeof(*arr)); \
      if (arr == NULL) { \
        err = TCALC_BAD_ALLOC; \
      } else { \
        capacity = 1; \
      } \
    } else if (size > capacity) { \
      size_t new_capacity = alloc_nr(capacity) < size ? size : alloc_nr(capacity); \
      void* realloced = realloc(arr, sizeof(*arr) * new_capacity); \
      if (realloced == NULL) { \
        err = TCALC_BAD_ALLOC; \
      } else { \
        arr = realloced; \
        capacity = new_capacity; \
      } \
    } \
  } while (0);

/**
 * arr: The pointer to the array to push val onto
 * size: The current size of the array
 * capacity: The current capacity of the array
 * val: The value to add to the array
 * err: a tcalc_error_t variable that will be set upon any errors
*/
#define TCALC_DARR_PUSH(arr, size, capacity, val, err) do { \
    TCALC_DARR_GROW(arr, size + 1, capacity, err) \
    if (err == TCALC_OK) { \
      arr[size++] = val; \
    } \
  } while (0);

/**
 * arr: The pointer to the array to perform possible growth on
 * size: The current size of the array
 * capacity: The current capacity of the array
 * val: The value to add to the array
 * index: The index to insert the value at
 * err: a tcalc_error_t variable that will be set upon any errors
*/
#define TCALC_DARR_INSERT(arr, size, capacity, val, index, err) do { \
    if (index > size || index < 0) { \
      err = TCALC_OUT_OF_BOUNDS; \
    } else { \
      TCALC_DARR_GROW(arr, size + 1, capacity, err) \
      if (err == TCALC_OK) { \
        size++; \
        for (size_t i = index + 1; i < size + 1; i++) { \
          arr[i] = arr[i - 1]; \
        } \
        arr[index] = val; \
      } \
    } \
  } while (0);

/**
 * Inspired from the ALLOC_GROW API In git
 * 
 * ALLOC_GROW Documentation: https://github.com/git/git/blob/35f6318d44379452d8d33e880d8df0267b4a0cd0/Documentation/technical/api-allocation-growing.txt#L1-L20
 * Also in https://github.com/git/git/blob/master/git-compat-util.h
*/
tcalc_error_t tcalc_alloc_grow(void** arr, size_t item_size, size_t size, size_t* capacity);

#endif