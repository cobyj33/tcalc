#ifndef TCALC_MEM_H
#define TCALC_MEM_H

#include "tcalc_error.h"

#include <stddef.h>
#include <malloc.h>

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

#define alloc_nr(x) (((x)+16)*3/2)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

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
 *
 *
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
    if ((capacity) == 0) { \
      (arr) = malloc(sizeof(*(arr))); \
      if ((arr) == NULL) { \
        (err) = TCALC_ERR_BAD_ALLOC; \
      } else { \
        (capacity) = 1; \
      } \
    } else if ((size) > (capacity)) { \
      size_t new_capacity = alloc_nr(capacity) < (size) ? (size) : alloc_nr(capacity); \
      void* realloced = realloc((arr), sizeof(*(arr)) * new_capacity); \
      if (realloced == NULL) { \
        err = TCALC_ERR_BAD_ALLOC; \
      } else { \
        (arr) = realloced; \
        (capacity) = new_capacity; \
      } \
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
    if (index > (size) || index < 0) { \
      err = TCALC_ERR_OUT_OF_BOUNDS; \
    } else { \
      TCALC_DARR_GROW(arr, (size) + 1, capacity, err); \
      if (err == TCALC_ERR_OK) { \
        size++; \
        for (size_t i = index + 1; i < (size) + 1; i++) { \
          (arr)[i] = (arr)[i - 1]; \
        } \
        (arr)[index] = (val); \
      } \
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

#endif
