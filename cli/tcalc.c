#include "tcalc.h"
#include "tcalc_eval.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>

const char* TCALC_HELP_MESSAGE = "tcalc usage: tcalc [-h] expression \n"
"\n"
"Optional arguments:\n"
"    -h --help: Show this help message\n";

int main(int argc, char** argv) {

  static struct option const longopts[] = {
    {"help", no_argument, NULL, 'h'},
  };

  int opt;
  int long_index;
  while ((opt = getopt_long(argc, argv, "h", longopts, &long_index)) != -1) {
    switch (opt) {
      case 'h': {
        fputs(TCALC_HELP_MESSAGE, stdout);
        return EXIT_SUCCESS;
      }
      default: {
        fputs(TCALC_HELP_MESSAGE, stderr);
        return EXIT_FAILURE;
      }
    }
  }

  if (optind >= argc) {
    fputs("tcalc: Expected expression argument\n", stderr);
    fputs(TCALC_HELP_MESSAGE, stderr);
    return EXIT_FAILURE;
  }

  double ans;
  tcalc_error_t err = tcalc_eval(argv[optind], &ans);
  if (err) {
    fprintf(stderr, "tcalc error: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
  }
  printf("%f\n", ans);

  return EXIT_SUCCESS;
}