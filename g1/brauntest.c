#include <stdlib.h>
#include <stdio.h>
#include "braunseq.h"

#define BASENUMBER 42
#define BASEADD addL
#define BASEREMV remvR

int main(int argc, char **argv) {
  int tests = BASENUMBER;
  void (*add)(Tree, Data) = BASEADD;
  Data (*remv)(Tree) = BASEREMV;
  bNode *tree[1];
  int i;

  tree[0] = NULL;

  if (argc >= 2) {
    tests = atoi(argv[1]);
  }
  if (argc >= 3) {
    if (*argv[2] == 'l' || *argv[2] == 'L')
      add = addL;
    else if (*argv[2] == 'r' || *argv[2] == 'R')
      add = addR;
  }
  if (argc >= 4) {
    if (*argv[3] == 'l' || *argv[3] == 'l')
      remv = remvL;
    else if (*argv[3] == 'r' || *argv[3] == 'R')
      remv = remvR;
  }


  if (add == addL)
    printf("Using addL\n");
  else
    printf("Using addR\n");
  if (remv == remvL)
    printf("Using remvL\n");
  else
    printf("Using remvR\n");

  for (i = 0; i < tests; i++)
    add(tree,(Data)i);
  printf("\n%i Nodes inserted\n",size(tree));
  //printTree(tree);
  printf("The following nodes have been inserted in order:\n");
  for (i = 0; i < tests; i++)
    printf("lookup: %lf\n",lookup(tree,i));
  printf("\n");
  for (i = 0; i < tests; i++)
    printf("%lf\n",remv(tree));
}
  
void printNode(bNode *node, int depth) {
  if (node == NULL)
    return;
  printf("depth:%i, data:%lf\n",depth,node->el);
  printNode(node->left,depth+1);
  printNode(node->right,depth+1);
}

void printTree(Tree tree) {
  printNode(*tree,0);
}
