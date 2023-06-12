#include <stdio.h>  // for getc, printf
#include <stdlib.h> // malloc, free
#include "ijvm.h"
#include "util.h" // read this file for debug prints, endianness helper functions
#include <string.h>
#include "assert.h"

// see ijvm.h for descriptions of the below functions

FILE *in;   // use fgetc(in) to get a character from in.
            // This will return EOF if no char is available.
FILE *out;  // use for example fprintf(out, "%c", value); to print value to out

uint32_t ctNum, txtNum, progCount; //How many bytes there are in Constant and Text respectively
int *ctVals, *txtVals;

#define maxSize 1024

struct Stack
{
  int items[maxSize];
  int top;
};

void initialize(struct Stack* stack) {
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
  assert(stack->top != (maxSize - 1) && "Stack overflow");
  stack->top +=1;
  stack->items[stack->top] = element;
  printf("The pushed element is %d\n", stack->items[stack->top]);
}

uint32_t top(struct Stack* stack)
{
  assert(stack->top != -1 && "Stack is empty");
  printf("The top is %d\n", stack->items[stack->top]);
  return stack->items[stack->top];
}

void set_input(FILE *fp) 
{ 
  in = fp; 
}

void set_output(FILE *fp) 
{ 
  out = fp; 
}

int init_ijvm(char *binary_path) 
{
  in = stdin;
  out = stdout;
  uint8_t buf[4];
  progCount = 0; 
 
  FILE *f = fopen(binary_path, "rb");
  if(f == NULL) {
    return -1;
  }

  printf("File located at '%s' opened\n", binary_path);
  
  // Magic Number
  fread(buf, sizeof(uint8_t), 4, f); //reads from f 1 byte at a time, the first 4 bytes
  uint32_t magicNum = read_uint32_t(buf);
  if(magicNum != 0x1DEADFAD)
  {
    printf("This is not an IJVM file\n");
    return -1;
  }
  
  //Skip past ct origin
  fread(buf, sizeof(uint8_t), 4, f);

  //Read constant
  fread(buf, sizeof(uint8_t), 4, f);
  ctNum = read_uint32_t(buf);
  
  ctNum = ctNum / 4; //divided by 4 since each constant is 4 bytes
  ctVals = (int *)calloc(ctNum, 4); //allocates a block of 'ctNum' values, each 4 bytes
  fread(ctVals, sizeof(uint32_t), ctNum, f); //reads each constant from the block
  for(int i = 0; i < ctNum; i++)
  {
    ctVals[i] = swap_uint32(ctVals[i]);
  }

  //Skip past txt origin 
  fread(buf, sizeof(uint8_t), 4, f);

  //Read txt
  fread(buf, sizeof(uint8_t), 4 ,f);
  txtNum = read_uint32_t(buf);
  
  txtVals = (int *)calloc(txtNum, 4);
  fread(txtVals, sizeof(uint8_t), txtNum, f); //reads a byte each time, stores it in txtVals
  
  fclose(f);
  return 0;
}

void destroy_ijvm(void) 
{
  free(txtVals);
  free(ctVals);
}

byte_t *get_text(void) 
{
  return txtVals;
}

unsigned int get_text_size(void) 
{
  return txtNum;
}

word_t get_constant(int i) 
{
  return(ctVals[i]);
}

unsigned int get_program_counter(void) 
{
  // TODO: implement me
  return 0;
}

word_t tos(void) 
{
  // this operation should NOT pop (remove top element from stack)
  // TODO: implement me
  return -1;
}

bool finished(void) 
{
  // TODO: implement me
  return false;
}

word_t get_local_variable(int i) 
{
  // TODO: implement me
  return 0;
}

void step(void) 
{
  // TODO: implement me

}

void run(void) 
{
  while (!finished()) 
  {
    step();
  }
}

byte_t get_instruction(void) 
{ 
  return get_text()[get_program_counter()]; 
}

// Below: methods needed by bonus assignments, see ijvm.h

//int get_call_stack_size(void) 
//{
   // TODO: implement me
//   return sp;
//}


// Checks if reference is a freed heap array. Note that this assumes that 
// 
//bool is_heap_freed(word_t reference) 
//{
   // TODO: implement me
// return 0;
//}
