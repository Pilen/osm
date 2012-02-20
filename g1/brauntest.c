#include <stdlib.h>
#include <stdio.h>
#include "braunseq.h"

#define BASENUMBER 42

int main(int argc, char **argv) {
  int tests = BASENUMBER;  
  bNode *tree[1];
  int i;

  tree[0] = NULL;

  if (argc >= 2) {
    tests = atoi(argv[1]);
  }

  for (i = 0; i < tests; i++)
    addL(tree,(Data)i);
  printf("\n%i Nodes inserted\n",size(tree));
  printTree(tree);
  for (i = 0; i < tests; i++)
    printf("lookup: %lf\n",lookup(tree,i));
  printf("\n");
  for (i = 0; i < tests; i++)
    printf("%lf\n",remvR(tree));
}
  
