#ifndef TCALC_MEM_H
#define TCALC_MEM_H

#include <stddef.h>

void tcalc_free_arr(void** arr, size_t size, void(*freecb)(void*));

#endif