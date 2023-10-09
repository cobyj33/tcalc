#include "tcalc_eval.h"
#include <malloc.h>

// tcalc_error_t tcalc_eval(const char* infix, double* out) {
//   char* rpn;
//   tcalc_error_t err = tcalc_infix_to_rpn(infix, &rpn);
//   if (err) { return err; }

//   err = tcalc_eval_rpn(rpn, out);
//   free(rpn);
//   return err;
// }