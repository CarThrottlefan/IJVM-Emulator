#include "stack.h"


struct Stack globalStack;
struct Stack* globalStack_ptr = &globalStack;

void initialize(struct Stack* stack) {
    stack->items = (word_t*) malloc(initSize * sizeof(word_t));
    stack->currMaxSize = initSize;
    stack->topAddr = -1; //initializes the stack top pointer to -1 when a stack is made  
}

word_t pop(struct Stack* stack) 
{
  assert(stack->topAddr != -1 && "Stack is empty");
  stack->topAddr = stack->topAddr - 1;
  return stack->items[stack->topAddr + 1];
}

void push(struct Stack* stack, word_t element)
{
  if(stack->topAddr >= stack->currMaxSize - 1)
  {
    stack->items = (word_t*) realloc(stack->items, stack->currMaxSize * 2 * sizeof(word_t)); // makes the size of the stack currMax * 2
    stack->currMaxSize *= 2;
  }
  if(stack->topAddr == -1)
  {
     lv_addr = 0; //points to the bottom of the frame(bottom of stack in main)
  }
  stack->topAddr += 1;
  stack->items[stack->topAddr] = element;
}
