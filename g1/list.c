#include <stdlib.h>
#include <stdio.h>
#include "list.h"

/* used if it's not possible to allocate more memory with malloc */
static void out_of_memory() {
  printf("Out of memory.\n");
  exit(EXIT_FAILURE);
}

void append(Listnode **start, Data elem) {
  Listnode *ptr;
  Listnode *temp = (Listnode *) malloc(sizeof(Listnode));
  if (temp == NULL) out_of_memory();
  temp->content = elem;
  temp->next = NULL;

  if (*start == NULL)
    prepend(start, elem);
  else {
    for (ptr = *start; ptr->next != NULL; ptr = ptr->next);
    ptr->next = temp;
  }
}

void prepend(Listnode **start, Data elem) {
  Listnode *temp = (Listnode *) malloc(sizeof(Listnode));
  if (temp == NULL) out_of_memory();
  temp->content = elem;
  temp->next = *start;
  *start = temp;
}

int length(Listnode *start) { 
  int len = 0;
  while (start != NULL) {
    len++;
    start = start->next;
  }
  return len;
}

Data head(Listnode *start) {
  if (start == NULL)
    return NULL;
  return start->content;
}

void remv(Listnode **start, int (*match)(Data))
{
  Listnode *curr, **last;
  if (*start == NULL) return;
  last = start;
  curr = *start;
  while (curr!=NULL && !(match(curr->content))) {
    last = &(curr->next);
    curr = curr->next;
  }
  if (curr == NULL)
    return; /* not found, do nothing */

  *last = curr->next; /* found, remove and free */
  free(curr);
}
