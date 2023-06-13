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

uint32_t ctNum, txtNum, progCount = 0;
int *ctVals, *txtVals;
bool isFinished = false;

//------------------------Stack implementation begins---------------
#define initSize 1024

struct Stack
{
  word_t *items;
  int topAddr;
  uint32_t currMaxSize;
};

struct Stack globalStack;
struct Stack* globalStack_ptr = &globalStack;

void initialize(struct Stack* stack) {
    stack->items = (word_t*) malloc(initSize * sizeof(word_t));
    stack->currMaxSize = initSize;
    stack->topAddr = -1; //initializes the stack pointer to -1 when a stack is made
}

void pop(struct Stack* stack) 
{
  assert(stack->topAddr != -1 && "Stack is empty");
  printf("The popped element is %d\n", stack->items[stack->topAddr]);
  stack->topAddr -= 1;
}

void push(struct Stack* stack, uint32_t element)
{
  if(stack->topAddr == stack->currMaxSize - 1)
  {
    printf("Stack is being resized\n");
    stack->items = (word_t*) realloc(stack->items, stack->currMaxSize * 2 * sizeof(word_t)); // makes the size of the stack currMax * 2
    stack->currMaxSize *= 2;
  }
  stack->topAddr += 1;
  stack->items[stack->topAddr] = element;
  printf("The pushed element is %d\n", stack->items[stack->topAddr]);
}
//------------------------Stack implementation ends---------------

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
  initialize(globalStack_ptr);
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
  printf("Progcount: %u\n", progCount);
  return progCount;
}

word_t tos(void) 
{
  assert(globalStack_ptr->topAddr != -1 && "Stack is empty");
  int32_t signedTop = (int) globalStack_ptr->items[globalStack_ptr->topAddr];
  return signedTop;
}

bool finished(void) 
{
  // TODO: implement me
  return isFinished; //return vlue of bool is finished, if true ends program
}

word_t get_local_variable(int i) 
{
  // TODO: implement me
  return 0;
}

void extractTop2Vals(int32_t *first, int32_t *second)
{
  first = tos();
  pop(globalStack_ptr);
  second = tos();
  pop(globalStack_ptr);
}

void step(void) 
{
  // TODO: implement me
  byte_t operation = *(get_text() + progCount);
  printf("Instruction code: %x\n", operation);
  int32_t firstVal, secondVal, opResult;
  switch (operation)
  {
    case OP_BIPUSH:
    {
      int8_t arg = *(get_text() + 1);
      push(globalStack_ptr, arg);
      progCount = 2 * sizeof(byte_t);
      break;
    }

    case OP_DUP:
    {
      int32_t newVal = tos();
      push(globalStack_ptr, newVal);        
      progCount += sizeof(byte_t);
      break;
    }

    case OP_IADD:
    {
      
      extractTop2Vals(&firstVal, &secondVal);

      opResult = firstVal + secondVal;
      push(globalStack_ptr, opResult);
      progCount += sizeof(byte_t);
      break;
    }

    case OP_IAND:
    {
      extractTop2Vals(&firstVal, &secondVal);

      opResult = firstVal & secondVal;
      push(globalStack_ptr, opResult);
      progCount += sizeof(byte_t);
      break;
    }

    case OP_IOR:
    {
      extractTop2Vals(&firstVal, &secondVal);

      opResult = firstVal | secondVal;
      push(globalStack_ptr, opResult);
      progCount += sizeof(byte_t);
      break;
    }

    case OP_ISUB:
    {
      extractTop2Vals(&firstVal, &secondVal);

      opResult = secondVal - firstVal;
      push(globalStack_ptr, opResult);
      progCount += sizeof(byte_t);
      break;
    }

    case OP_NOP:
    {
      progCount += sizeof(byte_t);
      break;
    }  
    
    case OP_POP:
    {
      pop(globalStack_ptr);
      progCount += sizeof(byte_t);
      break;
    }

    case OP_SWAP:
    {
      extractTop2Vals(&firstVal, &secondVal);
      push(globalStack_ptr, firstVal);
      push(globalStack_ptr, secondVal);
      progCount += sizeof(byte_t);
      break;
    }

    case OP_ERR:
    {
      printf("An error has ocurred. Emulator halted\n");
      progCount += sizeof(byte_t);
    }

    case OP_HALT:
    {
      progCount += sizeof(byte_t);
      isFinished = true;
      finished();
    }

    case OP_OUT:
    {
      fgetc(in);
      progCount += sizeof(byte_t);
      break;
    }

    default:
      isFinished = true;
      finished();
      break;
  }
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
