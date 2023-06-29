#ifndef STACK_H
#define STACK_H

#include "ijvm.h"
#include "global_data.h"
#include <stdlib.h>
#include <assert.h>

#define initSize 10024

struct Stack
{
  word_t *items;
  int topAddr;
  uint32_t currMaxSize;
};

/*struct Heap_array
{
  word_t *arrays; // a thing which stores all the arrays, an array per index()
  word_t *items;
  uint32_t size;
};

void initialize_heap(struct Heap_array heap);*/

//extern struct Stack globalStack;
extern struct Stack* globalStack_ptr;

void initialize(struct Stack* stack);

word_t pop(struct Stack* stack);

void push(struct Stack* stack, word_t element);
#endif // STACK_H
