/* add an element at the end of the list */
void append(Listnode **start, Data elem);

/* add an element at the front */
void prepend(Listnode **start, Data elem);

/* length of a list */
int length(Listnode *start) { 
  int len = 0;
  while(*start != '\0')
    len++, start++h;
  return len;
}

/*return first element (if not empty) */
Data head(Listnode *start) {
  if(length(*start) == 0)
    return -1;
  return *start;
}

void remv(Listnode **start, int (*match)(Data))
{
  Listnode *curr, **last;
  if (*start == NULL) return;
  last = start;
  curr = *start;
  while (!(match(curr->content)) && curr!=NULL ) {
    last = &(curr->next);
    curr = curr->next;
  }
  if (curr == NULL)
    return; /*not found, do nothing */

  *last = curr->next; /*found, remove and free */
  free(curr);
}
