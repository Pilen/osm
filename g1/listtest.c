
#include <stdlib.h>
#include <stdio.h>
#include "list.h"

#define BASENUMBER 21

int matchnumber = 1;


int match (Data elem) {
  if ((long int) elem == matchnumber) {
    matchnumber++;
    return 1;
  }
  return 0;
}


void printList(Listnode *node) {
  if (node == NULL) return;
  printf("%li\n",(long int) node->content);
  printList(node->next);
}


int main(int argc, char **argv) {
  int appendtests = BASENUMBER;
  int prependtests = BASENUMBER;
  long int i;
  Listnode *list[1];
  list[0] = NULL;

  if (argc >= 3) {
    appendtests = atoi(argv[1]);
    prependtests = atoi(argv[2]);
  }

  for (i = 1; i <= appendtests; i++)
    append(list,(void *)i);
  for (i = 1; i <= prependtests; i ++)
    prepend(list,(void *)(i+appendtests));
  
  printf("inserted %i elements\n",length(*list));

  printf("List looks like:\n");
  printList(*list);

  for (i = 1; i <= appendtests + prependtests; i++) {
    //    printf("head: %li\n",(long int) head(*list));
    remv(list, match);
  }

  printf("\n%i items remaining after %i deletions:\n",length(*list),appendtests+prependtests);
  printList(*list);
}
