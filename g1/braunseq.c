#include <stdlib.h> // standard library, defining NULL, malloc, free
#include <stdio.h>  // standard I/O functions

// data types and interface
#include "braunseq.h"

/*
 * internal helper functions
 *
 ****************************/
void error(char* msg) { // report an error and exit
  printf("PROGRAM ERROR: %s\n", msg);
  exit(EXIT_FAILURE);
}

// divide by 2 using bit shift
static inline int half(int i) { return (i>>1); }
// test if an int is odd (return 1 if it is, 0 otherwise)
static inline int odd(int i) { return (i & 0x1); }

// recursive helper for size: difference btw. tree size and m (recursive)
int diff(int n, bNode* t);
// insert at last position, knowing the tree size i >= 0 (for recursion)
void ins(Tree tree, int i, Data el);
// combine subtrees t1 and t2 into t1. Assumes size t1 same or 1 bigger than t2
void combine(Tree t1, Tree t2);
// delete last element, knowing the tree size i >= 0 (for recursion)
void del(Tree tree, int i);

int size(Tree tree) {
  if (*tree == NULL) {
    return 0;
  } else {
    int sz = size(&((*tree)->right));
    return (1 + 2*sz + diff(sz, (*tree)->left));
  }
}

void rebalance(bNode *);
void addL(Tree tree, Data new_el){
  // insert into new root, push old root into right subtree, swap sides
  bNode *new = (bNode *) malloc(sizeof(bNode));
  new->el = new_el;
  new->left = NULL;
  new->right = NULL;

  if (*tree == NULL) {
    *tree = new;
    return;
  }

  printf("jeg er ikke et null\n");

  new->right = (*tree)->left;
               (*tree)->left = NULL;
  new->left  = (*tree);
  rebalance(new->left);
  *tree = new;
  return;
}

void rebalance(bNode *top) {
  /*
  if (top->left == NULL) {
    top->left = top->right;
    top->right = NULL;
    return;
  }
  */
  if (top->right == NULL)
    return;
  top->left = top->right;
  top->right = top->left->left;
               top->left->left = NULL;
  rebalance (top->left);
}


void addR(Tree tree, Data new_el) {
  // navigate to last element (use size), insert there
  ins(tree, size(tree), new_el);
}

Data remvL(Tree tree) {
  // remove root (use "combine" helper), return removed element
  bNode old_root = **tree; // make a copy
  free(*tree); // return unused memory
  *tree = old_root.left;
  combine(tree, &old_root.right);
  return old_root.el;
}

Data remvR(Tree tree) {
  // use size to navigate through the tree recursively (helper del)
  // before deleting, call lookup to retrieve the data that is returned
  int tsize = size(tree);
  Data result = lookup(tree,tsize-1);
  del(tree,tsize-1);
  return result;
}

Data lookup(Tree tree, int i) {
  // navigate through tree recursively (i odd/even)
  if (*tree == NULL || i < 0) {
    error("lookup: index out of bounds");
  }
  if (i == 0) {
    return (*tree)->el;
  }
  if (odd(i)) {
    return ( lookup(&((*tree)->left), half(i)) );
  } else {
    return ( lookup(&((*tree)->right), half(i)-1) );
  }
}

/* ************************ 
 * Implementation internals
 *
 **************************/

// helper for size: difference btw. tree size and m (recursive, O(log n))
int diff(int n, bNode* t) {
  if (n == 0) { return (t == NULL ? 0 : 1); }
  // n != 0
  if (t == NULL) { error("diff encountered an internal error."); }
  if (odd(n)) {
    return diff(half(n), t->left);
  } else {
    return diff(half(n)-1, t->right);
  }
}

// insert at last position, knowing size is i >= 0 (recursive)
void ins(Tree tree, int i, Data el) {
  if (i < 0) { error("ins received invalid data."); }
  if (i == 0) {
    bNode* node;
    if (*tree != NULL) { error("ins expected tree==NULL"); }
    node = malloc(sizeof(bNode));
    if (node == NULL) { error("insert: failed to malloc"); }
    node->el = el;
    node->left = NULL;
    node->right=NULL;
    *tree = node;
    return;
  }
  if (odd(i)) { // if i odd: insert into left subtree
    ins(&((*tree)->left), half(i), el);
  } else {      // if even: insert into right subtree
    ins(&((*tree)->right), half(i)-1, el);
  }
}

// combine two subtrees into one. assumes size t1 same or 1 bigger than t2
void combine(Tree t1, Tree t2) {
  if (*t1 == NULL) { return; // t1 empty, so is t2. Nothing to do.
  } else {
    // otherwise, the new root is the t1 root, with left subtrees t2 
    // and right subtree  combined from t1 subtrees (recursively)
    combine(&((*t1)->left), &((*t1)->right));
    (*t1)->right = (*t1)->left;
    (*t1)->left = *t2;
  }
}

// delete last element, knowing the size is i >= 0 (recursive)
void del(Tree tree, int i) {
  // navigate through the tree to find the last element (i is size).
  if (i < 0) { error("del recieved invalid index");}
  if (i == 0) {
    free(*tree);
    return;
  }
  if (i == 1) {
    free((*tree)->left);
    (*tree)->left = NULL;
    return;
  }
  if (i == 2) {
    free((*tree)->right);
    (*tree)->right = NULL;
    return;
  }
  if (odd(i)) { // if i odd: delete from left subtree
    del(&((*tree)->left), half(i));
  } else {      // if even: delete from right subtree
    del(&((*tree)->right), half(i)-1);
  }

  return;
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
