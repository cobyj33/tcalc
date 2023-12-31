#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "tcalc.h"
#include "tcalc_eval.h"


int main(int argc, char** argv) {
  if (argc < 2) {
    printf("%s", "Insert 1 argument to be rpn evaluated\n");
    return EXIT_FAILURE;
  }

  double ans;
  tcalc_error_t err = tcalc_eval_rpn(argv[1], &ans);
  if (err) {
    printf("TCalc Error Occured: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
  }
  printf("%f\n", ans);

  return EXIT_SUCCESS;
}