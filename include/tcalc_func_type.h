#ifndef TCALC_FUNC_TYPE_H
#define TCALC_FUNC_TYPE_H

#include "tcalc_error.h"

typedef tcalc_error_t (*tcalc_unary_func)(double, double*);
typedef tcalc_error_t (*tcalc_binary_func)(double, double, double*);

#endif