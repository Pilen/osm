#include <stdlib.h>
#include "stack.h"

void stack_init(stack_t* stack) {
  stack = stack;
}

int stack_empty(stack_t* stack) {
  return length(*stack) == 0;
}

void* stack_top(stack_t* stack) {
  return head(*stack);
}

void* stack_pop(stack_t* stack) {
  void *result = stack_top(stack);

  Listnode *next = (**stack).next;
  free(*stack);
  *stack = next;

  return result;
}

int stack_push(stack_t* stack, void* data) {
  prepend(stack,data);
  return 0;
}
