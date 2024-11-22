#ifndef TCALC_MEM_H
#define TCALC_MEM_H

#include "tcalc_error.h"

#include <stddef.h>
#include <stdlib.h>

/**
 * Notes about memory:
 *
 * All freeing functions should be defined to take a NULL pointer as a valid
 * argument. Freeing functions should never fail and therefore should never
 * return a tcalc_err value. This greatly simplifies code using any freeing
 * function since there is no null-checks that have to be used by other code
 * before using freeing functions, and the cleanup: label pattern can work
 * from anywhere within the function after allocated or soon-to-be-allocated
 * variables have been defined.
 *
 * Most special types have specific alloc and free functions. For simplicity,
 * and at least as of now, all types which have specific alloc and free functions
 * should only be constructed using these functions.
*/

#define TCALC_ALLOC_NR(x) (((x / 2) + 8) * 3)
#define TCALC_ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
 * Extended malloc function which aborts the program on malloc failure
 *
 * Note that this may not be the best way to handle malloc failures in general
 * code, as interactive sessions could terminate unexpectedly which may have
 * been truly recoverable with more graceful methods.
 *
 * @param size the size in bytes that the allocated memory block should be
 * @returns A guaranteed non-NULL pointer to the allocated memory block
*/
void* tcalc_xmalloc(size_t);

/**
 * Extended calloc function which aborts the program on malloc failure
 *
 * Note that this may not be the best way to handle malloc failures in general
 * code, as interactive sessions could terminate unexpectedly which may have
 * been truly recoverable with more graceful methods.
 *
 * @param nmemb the number of members to allocate
 * @param memsize the size of a single member of the type wanted to be allocated
 * @returns A guaranteed non-NULL pointer to the allocated memory block of the
 * size memsize * nmemb. All of the bytes in the function will be initialized to 0.
*/
void* tcalc_xcalloc(size_t, size_t);
/**
 * Extended realloc function which aborts the program on malloc failure
 *
 * Note that this may not be the best way to handle malloc failures in general
 * code, as interactive sessions could terminate unexpectedly which may have
 * been truly recoverable with more graceful methods.
 *
 * @param ptr The pointer that should be reallocated. This pointer could be
 * allocated with the C standard malloc or tcalc_xmalloc
 * @param size the size in bytes that the new memory block should be
 * @returns A guaranteed non-NULL pointer to the allocated memory block
*/
void* tcalc_xrealloc(void*, size_t);

/**
 * arr: The pointer to the array to perform possible growth on
 * size: The size to which the array should fit into
 * capacity: The current capacity of the array
 * err: a tcalc_err variable that will be set upon any errors
 *
 * the capacity variable will be updated to the current array's capacity if
 * reallocated, or will stay the same under no reallocation
 *
 * the arr variable will be altered, if reallocated, to point to the reallocated
 * data
*/
#define TCALC_DARR_GROW(arr, size, capacity, err) do { \
    if ((size) > (capacity)) { \
      const size_t grow_func_res = TCALC_ALLOC_NR(capacity); \
      if (grow_func_res < (size)) { \
        err = TCALC_ERR_OVERFLOW; \
      } else { \
        const size_t new_capacity = grow_func_res < (size) ? (size) : grow_func_res; \
        void* realloced = realloc((arr), sizeof(*(arr)) * new_capacity); \
        if (realloced == NULL) { \
          err = TCALC_ERR_BAD_ALLOC; \
        } else { \
          (arr) = realloced; \
          (capacity) = new_capacity; \
        } \
      } \
    } else { \
      err = TCALC_ERR_OK; \
    } \
  } while (0)

/**
 * arr: The pointer to the array to push val onto
 * size: The current size of the array
 * capacity: The current capacity of the array
 * val: The value to add to the array
 * err: a tcalc_err variable that will be set upon any errors
*/
#define TCALC_DARR_PUSH(arr, size, capacity, val, err) do { \
    TCALC_DARR_GROW(arr, (size) + 1, capacity, err); \
    if ((err) == TCALC_ERR_OK) { \
      (arr)[(size)++] = (val); \
    } \
  } while (0)

/**
 * arr: The pointer to the array to perform possible growth on
 * size: The current size of the array
 * capacity: The current capacity of the array
 * val: The value to add to the array
 * index: The index to insert the value at
 * err: a tcalc_err variable that will be set upon any errors
*/
#define TCALC_DARR_INSERT(arr, size, capacity, val, index, err) do { \
    assert((index) < (size) && (index) >= 0); \
    TCALC_DARR_GROW(arr, (size) + 1, capacity, err); \
    if ((err) == TCALC_ERR_OK) { \
      (size)++; \
      for (size_t i = (size) - 1; i > (index); i--) { \
        (arr)[i] = (arr)[i - 1]; \
      } \
      (arr)[index] = (val); \
    } \
  } while (0)

/**
 * Free the contents of an array with dynamically allocated members with a callback
 *
 * Free the contents of the array with a specific freeing function. The freeing
 * function should take one pointer which is the same type as that stored in the
 * array,
*/
#define TCALC_ARR_FREE_CF(arr, len, freefn) do { \
  for (size_t i = 0; i < (len); i++) { \
    freefn((arr)[i]); \
  } \
} while (0)

/**
 * Free the contents of an array with dynamically allocated members with a callback
 *
 * This is different from TCALC_ARR_FREE_CF, since the members will be casted to void*
 * first when feeding into the free function
 *
 * the signautre of the freeing function takes one void pointer as an argument
 * and returns null (the same as the C standard free function)
*/
#define TCALC_ARR_FREE_CFV(arr, len, freefnv) do { \
  for (size_t i = 0; i < (len); i++) { \
    freefnv((void*)((arr)[i])); \
  } \
} while (0)

/**
 * Free a dynamically allocated array and all of its contents, freeing the contents with a callback.
*/
#define TCALC_ARR_FREE_F(arr, len, freefn) do { \
    TCALC_ARR_FREE_CF(arr, len, freefn); \
    free(arr); \
    (arr) = NULL; \
  } while (0)

/**
 * Free a dynamically allocated array and all of its contents, freeing the
 * contents with a callback where each member of the array will first be
 * casted to void*.
*/
#define TCALC_ARR_FREE_FV(arr, len, freefnv) do { \
    TCALC_ARR_FREE_CFV(arr, len, freefnv); \
    free(arr); \
    (arr) = NULL; \
  } while (0)

#define TCALC_VEC(type) struct { \
    type* arr; \
    size_t len; \
    size_t cap; \
  }

#define TCALC_VEC_INIT { .arr = NULL, .len = 0, .cap = 0 }
#define TCALC_VEC_FREE(vec) do { \
    free((vec).arr); \
    (vec).arr = NULL; \
    (vec).len = 0; \
    (vec).cap = 0; \
  } while (0)


#define TCALC_VEC_PUSH(vec, item, err) TCALC_DARR_PUSH(vec.arr, vec.len, vec.cap, item, err)
#define TCALC_VEC_INSERT(vec, item, index, err) TCALC_DARR_INSERT(vec.arr, vec.len, vec.cap, item, index, err)
#define TCALC_VEC_GROW(vec, res_size, err) TCALC_DARR_GROW(vec.arr, res_size, vec.cap, err)
#define TCALC_VEC_FOREACH(vec, iname) for (size_t iname = 0; iname < vec.len; iname++)

#define TCALC_VEC_FREE_F(vec, freefn) do { \
  TCALC_ARR_FREE_F(vec.arr, vec.len, freefn); \
  TCALC_VEC_FREE(vec); \
} while (0)

#define TCALC_VEC_FREE_FV(vec, freefnv) do { \
  TCALC_ARR_FREE_F(vec.arr, vec.len, freefnv); \
  TCALC_VEC_FREE(vec); \
} while (0)


#endif
