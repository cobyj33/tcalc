#include "tcalc_exit.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void tcalc_die(const char *err, ...) {
	char msg[4096];
	va_list params;
	va_start(params, err);
	vsnprintf(msg, sizeof(msg), err, params);
	fprintf(stderr, "%s\n", msg);
	va_end(params);
	exit(1);
}
