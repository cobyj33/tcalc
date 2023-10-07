#ifndef TCALC_PARSETREE_H
#define TCALC_PARSETREE_H

#include <stddef.h>

struct ParseTreeNode {
  char* token;

  struct ParseTreeNode* children;
  size_t nb_children;
}

#endif