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
bool isFinished = false, sameFunc = false, wide = false, methodCount = false;
int16_t opOffset, lv_addr, opOffset_cpy;

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
    stack->topAddr = -1; //initializes the stack top pointer to -1 when a stack is made  
}

word_t pop(struct Stack* stack) 
{
  assert(stack->topAddr != -1 && "Stack is empty");
  stack->topAddr = stack->topAddr - 1;
  return stack->items[stack->topAddr + 1];
}

void push(struct Stack* stack, uint32_t element)
{
  if(stack->topAddr == stack->currMaxSize - 1)
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
  isFinished = false;
  sameFunc = false;
  uint8_t buf[4];
  progCount = 0; 
 
  FILE *f = fopen(binary_path, "rb");
  if(f == NULL) {
    return -1;
  }

  printf("\nFile located at '%s' opened\n\n", binary_path);
  
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
  return (byte_t *)txtVals;
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
  return isFinished; //return value of bool is finished, if true ends program
}

word_t get_local_variable(int i) 
{
  return globalStack_ptr -> items[lv_addr + i];
}

void step(void) 
{
  byte_t operation = *(get_text() + progCount);
  int32_t firstVal, secondVal, opResult;

  if(lv_addr == 0 && !sameFunc)
  {
    opOffset = 256; //the space allocated for variables, for main it is 256
    sameFunc = true;
    globalStack_ptr -> topAddr += opOffset;
  }

for(;;)
{
  switch (operation)
  {
    case OP_BIPUSH:
    {
      int8_t arg = *(get_text() + progCount + sizeof(byte_t));
      push(globalStack_ptr, arg);
      progCount += 2 * sizeof(byte_t);
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
      firstVal = pop(globalStack_ptr);

      secondVal = pop(globalStack_ptr);

      opResult = firstVal + secondVal;
      push(globalStack_ptr, opResult);
      progCount += sizeof(byte_t);
      break;
    }

    case OP_IAND:
    {
      firstVal = pop(globalStack_ptr);
      secondVal = pop(globalStack_ptr);

      opResult = firstVal & secondVal;
      push(globalStack_ptr, opResult);
      progCount += sizeof(byte_t);
      break;
    }

    case OP_IOR:
    {
      firstVal = pop(globalStack_ptr);
      secondVal = pop(globalStack_ptr);

      opResult = firstVal | secondVal;
      push(globalStack_ptr, opResult);
      progCount += sizeof(byte_t);
      break;
    }

    case OP_ISUB:
    {
      firstVal = pop(globalStack_ptr);
      secondVal = pop(globalStack_ptr);

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
      firstVal = pop(globalStack_ptr);
      secondVal = pop(globalStack_ptr);

      push(globalStack_ptr, firstVal);
      push(globalStack_ptr, secondVal);
      progCount += sizeof(byte_t);
      break;
    }


    case OP_OUT:
    {
      firstVal = pop(globalStack_ptr);
      fprintf(out, "%c", firstVal);
      progCount += sizeof(byte_t);
      break;
    }

    case OP_IN:
    {
      firstVal = fgetc(in);
      if(firstVal != EOF)
      {
        push(globalStack_ptr, firstVal);
      }
      else
      {
        push(globalStack_ptr, 0);
      }  
      progCount += sizeof(byte_t);
      break;
    }

    case OP_GOTO:
    {
      int16_t offset = read_uint16_t(get_text() + progCount + 1);
      progCount += offset;
      break;
    }

    case OP_IFEQ:
    {
      int16_t offset = read_uint16_t(get_text() + progCount + 1);
      firstVal = pop(globalStack_ptr);
      if(firstVal == 0)
      {
        progCount += offset;
      }
      else
      {
        progCount += 1 + sizeof(offset);
      }
      break;
    }

    case OP_IFLT:
    {
      int16_t offset = read_uint16_t(get_text() + progCount + 1);
      firstVal = pop(globalStack_ptr);
      if(firstVal < 0)
      {
        progCount += offset;
      }
      else
      {
        progCount += 1 + sizeof(offset);
      }
      break;
    }

    case OP_IF_ICMPEQ:
    {
      int16_t offset = read_uint16_t(get_text() + progCount + 1);
      firstVal = pop(globalStack_ptr);
      secondVal = pop(globalStack_ptr);
      if(firstVal == secondVal)
      {
        progCount += offset;
      }
      else
      {
        progCount += 1 + sizeof(offset);
      }
      break;
    }

    case OP_ERR:
    {
      printf("An error has ocurred. Emulator halted\n");
      isFinished = true;
      break;
    }

    case OP_HALT:
    {
      isFinished = true;
      break;
    }

    case OP_LDC_W:
    {
      int16_t arg = read_uint16_t(get_text() + progCount + 1);
      word_t constant = get_constant(arg);
      push(globalStack_ptr, constant);
      progCount += 1 + sizeof(short);
      break;
    }

    case OP_ILOAD:
    {
      uint16_t arg;
      if(!wide)
      {
        arg = (uint16_t) *(get_text() + progCount + sizeof(byte_t));
        progCount += sizeof(byte_t) + 1;
      }
      else
      {
        arg = read_uint16_t(get_text() + progCount + 1);
        wide = false;
        progCount += sizeof(short) + 1;
      }

      word_t var = get_local_variable(arg);
      push(globalStack_ptr, var);
      break;
    }

    case OP_ISTORE:
    {
      word_t val = pop(globalStack_ptr);
      uint16_t arg;

      if(!wide)
      {
        arg = (uint16_t) *(get_text() + progCount + sizeof(byte_t));
        progCount += sizeof(byte_t) + 1;
      }
      else
      {
        arg = read_uint16_t(get_text() + progCount + 1);
        wide = false;
        progCount += sizeof(short) + 1;
      }

      globalStack_ptr -> items[lv_addr + arg] = val;
      break;
    }

    case OP_IINC:
    {
      uint16_t arg; 
      int16_t val;
      
      if(!wide)
      {
        arg = (uint16_t) *(get_text() + progCount + sizeof(byte_t));
        int8_t temp = *(get_text() + progCount + 2 * sizeof(byte_t));
        val = (int16_t) temp;
        progCount += 2 * sizeof(byte_t) + 1 ;
      }
      else
      {
        arg = read_uint16_t(get_text() + progCount + 1);
        int8_t temp = *(get_text() + progCount + sizeof(short) + 1);
        val = (int16_t) temp;
        wide = false;
        progCount += sizeof(byte_t) + sizeof(short) + 1;
      }
      
      globalStack_ptr -> items[lv_addr + arg] += val;
      break;
    }

    case OP_WIDE:
    {
      wide = true;
      progCount += sizeof(byte_t);
      operation = *(get_text() + progCount);
      continue;
    }

    case OP_INVOKEVIRTUAL:
    {
      word_t callerPC, callerLV;

      int16_t indexOfConstant = read_uint16_t(get_text() + progCount + 1); 
      int32_t startMethodArea = get_constant(indexOfConstant);
      uint32_t numOfArgs = read_uint16_t(get_text() + startMethodArea); 
      uint32_t numOfVars = read_uint16_t(get_text() + sizeof(short) + startMethodArea);
      
      callerPC = progCount;
      callerLV = lv_addr;
      progCount = 2 * sizeof(short) + startMethodArea; //first instruction is 5 bytes in
      opOffset_cpy = globalStack_ptr -> topAddr;

      lv_addr = globalStack_ptr -> topAddr - numOfArgs + 1; //gets the addres of LV in the stack
      int varOffset = lv_addr + numOfArgs; //gets the address where the local vars are stored

      opOffset = varOffset + numOfVars; //offset for the operations in the stack
      globalStack_ptr -> topAddr = opOffset - 1;

      push(globalStack_ptr, callerPC);
      globalStack_ptr -> items[lv_addr] = globalStack_ptr -> topAddr;
      push(globalStack_ptr, callerLV);

      break;
    }

    case OP_IRETURN:
    {
      if(lv_addr == 0)
      {
        isFinished = true; //halts the program
      }

      word_t callerPC, callerLV, returnVal;
      
      returnVal = pop(globalStack_ptr);

      word_t lv = globalStack_ptr -> items[lv_addr];
      callerLV = globalStack_ptr -> items[lv+1];
      callerPC = globalStack_ptr -> items[lv];

      globalStack_ptr -> topAddr = lv_addr - 1;
      lv = callerLV;
      lv_addr = callerLV;

      progCount = callerPC;
      opOffset = opOffset_cpy;

      push(globalStack_ptr, returnVal);
      progCount += sizeof(short) + sizeof(byte_t);
      break;
    }

    case OP_NEWARRAY:
    {
      int32_t count = pop(globalStack_ptr);
      word_t *array_ptr;
      array_ptr = (word_t*) malloc(count * sizeof(word_t));

    }

    default:
      isFinished = true;
      break;
  }
  break;
}

  if(progCount >= get_text_size())
  {
    isFinished = true;
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
