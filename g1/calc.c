#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

typedef double (*binop)(double,double);

static void out_of_memory() {
  printf("Out of memory.\n");
  exit(EXIT_FAILURE);
}

static void perform_binop(stack_t* stack, binop op) {
  double *x;
  double *y;
  double *z;
  if ((z = (double*)malloc(sizeof(double))) == NULL) {
    out_of_memory();
  }
  if (!stack_empty(stack)) {
    x = stack_pop(stack);
    if (!stack_empty(stack)) {
      y = stack_pop(stack);
      *z = (*op)(*x,*y);
      free(x);
      free(y);
      if (stack_push(stack, z) == 1) {
        out_of_memory();
      }
      return;
    } else {
      if (stack_push(stack, x) == 1) {
        out_of_memory();
      }
    }
  }
  free(z);
  printf("Too few elements on stack.\n");
}

static double plus(double x, double y) {
  return x+y;
}

static double sub(double x, double y) {
  return x-y;
}

static double mult(double x, double y) {
  return x*y;
}

static double divn(double x, double y) {
  return x/y;
}

int main() {
  double x;
  int c;
  stack_t stack;
  stack_init(&stack);
  while (1) {
    c=getc(stdin);
    switch (c) {
    case EOF:
      exit(EXIT_SUCCESS);
    case '+':
      perform_binop(&stack, &plus);
      break;
    case '-':
      perform_binop(&stack, &sub);
      break;
    case '*':
      perform_binop(&stack, &mult);
      break;
    case '/':
      perform_binop(&stack, &divn);
      break;
    case 'p':
      if (stack_empty(&stack)) {
        printf("Stack is empty.\n");
      } else {
        printf("%lf\n", *(double*)stack_top(&stack));
      }
      break;
    default:
      ungetc(c,stdin);
      if (scanf("%lf",&x) == 1) {
        double* d;
        if ((d = (double*)malloc(sizeof(double))) == NULL) {
          out_of_memory();
        } else {
          *d = x;
          if (stack_push(&stack,d) != 0) {
            out_of_memory();
          }
        }
      } else {
        printf("Bad input.\n");
      }
    }
    while (getc(stdin)!='\n');
  }
}
