#include "tcalc.h"
#include "tcalc_eval.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

const char* TCALC_HELP_MESSAGE = "tcalc usage: tcalc [-h] expression \n"
"\n"
"Optional arguments:\n"
"    -h --help: Show this help message\n";

int tcalc_repl();

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

  if (optind >= argc) return tcalc_repl();

  double ans;
  tcalc_error_t err = tcalc_eval(argv[optind], &ans);
  if (err) {
    fprintf(stderr, "tcalc error: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
  }
  printf("%f\n", ans);

  return EXIT_SUCCESS;
}

int str_in_list(const char* input, const char** list, int count) {
  for (int i = 0; i < count; i++) {
    if (strcmp(input, list[i]) == 0) return 1;
  }
  return 0;
}

const char* repl_entrance_text = "tcalc REPL begun: Enter q to exit\n";

int tcalc_repl() {
  fputs(repl_entrance_text, stdout);
  char input_buffer[4096] = {'\0'};
  const char* quit_strings[3] = {"quit", "q", "exit"};
  
  while (!str_in_list(input_buffer, quit_strings, 3)) {
    char* input = fgets(input_buffer, 4096, stdin);
    if (input == NULL) {
      fprintf(stderr, "Error reading from stdin: Exiting\n");
      return EXIT_FAILURE;
    }

    input[strcspn(input, "\r\n")] = '\0';
    if (str_in_list(input, quit_strings, 3)) break;

    double ans;
    tcalc_error_t err = tcalc_eval(input, &ans);
    if (err) {
      fprintf(stderr, "tcalc error: %s\n\n", tcalc_strerrcode(err));
    } else {
      printf("%f\n\n", ans);
    }
  }


}