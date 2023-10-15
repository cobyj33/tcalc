#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "tcalc_exprtree.h"

#define MAX_STACK_SIZE 500

void tcalc_exprtree_print(tcalc_exprtree_t* node, size_t depth);

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("%s\n", "Insert 1 argument to be  evaluated");
    return EXIT_FAILURE;
  }

  tcalc_exprtree_t* tree;
  tcalc_error_t err = tcalc_create_exprtree_infix(argv[1], &tree);
  if (err) {
    printf("TCalc Error Occured: %s\n ", tcalc_strerrcode(err));
    return EXIT_FAILURE;
  }

  tcalc_exprtree_print(tree, 0);

  tcalc_exprtree_free(tree);
  return EXIT_SUCCESS;
}


void tcalc_exprtree_print(tcalc_exprtree_t* node, size_t depth) {
  for (int i = 0; i < depth; i++)
    printf("|___");
  printf("%s\n", node->token->value);

  for (size_t i = 0; i < node->nb_children; i++) {
    tcalc_exprtree_print(node->children[i], depth + 1);
  }
}