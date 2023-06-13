/*#include <stdint.h>  
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


#define initSize 1

struct Stack
{
  int *items;
  int top;
  int currMaxSize;
};

void initialize(struct Stack* stack) {
    stack->items = (int*) malloc(initSize * sizeof(int));
    stack->currMaxSize = initSize;
    stack->top = -1; //initializes the stack pointer to -1 when a stack is made
}

void pop(struct Stack* stack) 
{
  assert(stack->top != -1 && "Stack is empty");
  printf("The popped element is %d\n", stack->items[stack->top]);
  stack->top -= 1;
}

void push(struct Stack* stack, uint32_t element)
{
  //assert(stack->top != (maxSize - 1) && "Stack overflow");
  if(stack->top == stack->currMaxSize - 1)
  {
    printf("Stack is being resized\n");
    stack->items = (int*) realloc(stack->items, stack->currMaxSize * 2); // makes the size of the stack currMax * 2
    stack->currMaxSize *= 2;
  }
  stack->top +=1;
  printf("Top addr : %u", stack->top);
  stack->items[stack->top] = element;
  printf("The pushed element is %d\n", stack->items[stack->top]);
}

uint32_t top(struct Stack* stack)
{
  assert(stack->top != -1 && "Stack is empty");
  printf("The top is %d\n", stack->items[stack->top]);
  return stack->items[stack->top];
}


int main(int argc, char** argv){
  struct Stack stack;
  struct Stack* stack_ptr = &stack;
  initialize(stack_ptr);
  //top(stack_ptr);
  //pop(stack_ptr);
  push(stack_ptr, 2);
  top(stack_ptr);
  push(stack_ptr, 3);
  top(stack_ptr);
  return 0;
}*/