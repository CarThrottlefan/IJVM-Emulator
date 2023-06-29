#include <stdio.h>  // for getc, printf
#include <stdlib.h> // malloc, free
#include "ijvm.h"
#include "util.h" // read this file for debug prints, endianness helper functions
#include <string.h>
#include "assert.h"
#include "stack.h"

// see ijvm.h for descriptions of the below functions

FILE *in;   // use fgetc(in) to get a character from in.
            // This will return EOF if no char is available.
FILE *out;  // use for example fprintf(out, "%c", value); to print value to out

uint32_t ctNum, txtNum, progCount = 0;
int *ctVals, *txtVals;
bool isFinished = false, sameFunc = false, wide = false, methodCount = false;
uint32_t opOffset, lv_addr, opOffset_cpy, heapSize = 256, numOfArrays = 0;
word_t *heap;

int stepCounter = 0;

uint64_t step_num = 1;

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
  for(uint32_t i = 0; i < ctNum; i++)
  {
    ctVals[i] = (int)swap_uint32((uint32_t)ctVals[i]);
  }

  //Skip past txt origin 
  fread(buf, sizeof(uint8_t), 4, f);

  //Read txt
  fread(buf, sizeof(uint8_t), 4 ,f);
  txtNum = read_uint32_t(buf);
  
  txtVals = (int *)calloc(txtNum, 4);
  fread(txtVals, sizeof(uint8_t), txtNum, f); //reads a byte each time, stores it in txtVals

  heap = malloc(sizeof(word_t*) * heapSize); //makes the heap, which contains the arrays
  
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
  return globalStack_ptr -> items[lv_addr + (uint32_t) i];
}

void step(void) 
{
  byte_t operation = *(get_text() + progCount);
  int64_t firstVal, secondVal, opResult;

  if(lv_addr == 0 && !sameFunc)
  {
    opOffset = 256; //the space allocated for variables, for main it is 256
    sameFunc = true;
    globalStack_ptr -> topAddr += opOffset;
  }

  for(;;) //looping due to WIDE, if there is a WIDE it gets back to the top then checks the next operation
  {
    switch (operation)
    {
      case OP_BIPUSH:
      {
        int8_t arg =(int8_t) *(get_text() + progCount + sizeof(byte_t));
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
        push(globalStack_ptr, (word_t) opResult);
        progCount += sizeof(byte_t);
        break;
      }

      case OP_IAND:
      {
        firstVal = pop(globalStack_ptr);
        secondVal = pop(globalStack_ptr);

        opResult = firstVal & secondVal;
        push(globalStack_ptr, (word_t) opResult);
        progCount += sizeof(byte_t);
        break;
      }

      case OP_IOR:
      {
        firstVal = pop(globalStack_ptr);
        secondVal = pop(globalStack_ptr);

        opResult = firstVal | secondVal;
        push(globalStack_ptr, (word_t) opResult);
        progCount += sizeof(byte_t);
        break;
      }

      case OP_ISUB:
      {
        firstVal = pop(globalStack_ptr);
        secondVal = pop(globalStack_ptr);

        opResult = secondVal - firstVal;
        push(globalStack_ptr, (word_t) opResult);
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

        push(globalStack_ptr, (word_t) firstVal);
        push(globalStack_ptr, (word_t) secondVal);
        progCount += sizeof(byte_t);
        break;
      }


      case OP_OUT:
      {
        firstVal = pop(globalStack_ptr);
        fprintf(out, "%c", (byte_t) firstVal);
        progCount += sizeof(byte_t);
        break;
      }

      case OP_IN:
      {
        firstVal = fgetc(in);
        if(firstVal != EOF)
        {
          push(globalStack_ptr, (word_t)firstVal);
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
        int16_t offset = (int16_t)read_uint16_t(get_text() + progCount + 1);
        progCount += (uint32_t) offset;
        break;
      }

      case OP_IFEQ:
      {
        int16_t offset = (int16_t) read_uint16_t(get_text() + progCount + 1);
        firstVal = pop(globalStack_ptr);
        if(firstVal == 0)
        {
          progCount =(uint32_t)((int32_t)progCount + offset); //TODO look at me
        }
        else
        {
          progCount += 1 + sizeof(offset);
        }
        break;
      }

      case OP_IFLT:
      {
        int16_t offset = (int16_t) read_uint16_t(get_text() + progCount + 1);
        firstVal = pop(globalStack_ptr);
        if(firstVal < 0)
        {
          progCount =(uint32_t)((int32_t)progCount + offset);
        }
        else
        {
          progCount += 1 + sizeof(offset);
        }
        break;
      }

      case OP_IF_ICMPEQ:
      {
        int16_t offset = (int16_t) read_uint16_t(get_text() + progCount + 1);
        firstVal = pop(globalStack_ptr);
        secondVal = pop(globalStack_ptr);
        if(firstVal == secondVal)
        {
          progCount =(uint32_t)((int32_t)progCount + offset);
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
        uint16_t arg = read_uint16_t(get_text() + progCount + 1);
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
          int8_t temp = (int8_t) *(get_text() + progCount + 2 * sizeof(byte_t));
          val = (int16_t) temp;
          progCount += 2 * sizeof(byte_t) + 1 ;
        }
        else
        {
          arg = read_uint16_t(get_text() + progCount + 1);
          int8_t temp = (int8_t) *(get_text() + progCount + sizeof(short) + 1);
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
        
        uint16_t indexOfConstant = read_uint16_t(get_text() + progCount + 1); 
        uint32_t startMethodArea = (uint32_t) get_constant(indexOfConstant);
        uint32_t numOfArgs = read_uint16_t(get_text() + startMethodArea); 
        uint32_t numOfVars = read_uint16_t(get_text() + sizeof(short) + startMethodArea);
        
        callerPC = (word_t) progCount;
        callerLV = (word_t) lv_addr;
        progCount = 2 * sizeof(short) + startMethodArea; //first instruction is 5 bytes in
        opOffset_cpy = (uint16_t) globalStack_ptr -> topAddr;

        lv_addr = ((uint32_t)globalStack_ptr -> topAddr - numOfArgs + 1); //gets the addres of LV in the stack
        uint32_t varOffset = (uint32_t) (lv_addr + numOfArgs); //gets the address where the local vars are stored

        opOffset = varOffset + numOfVars; //offset for the operations in the stack
        globalStack_ptr -> topAddr = (int32_t) (opOffset - 1);
        bool loop = false;
        
        while(globalStack_ptr->currMaxSize < opOffset)
        {
            globalStack_ptr->currMaxSize *= 2;
            loop = true;
        }
        if(loop)
        {
          globalStack_ptr->items = (word_t*) realloc(globalStack_ptr->items, globalStack_ptr->currMaxSize * 2 * sizeof(word_t));
          loop = false;
        }

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

        word_t linkPtr = globalStack_ptr -> items[lv_addr];
        callerLV = globalStack_ptr -> items[linkPtr+1];
        callerPC = globalStack_ptr -> items[linkPtr];

        globalStack_ptr -> topAddr = (int32_t) lv_addr - 1;
        linkPtr = callerLV;
        lv_addr = (uint32_t) callerLV;

        progCount = (uint32_t) callerPC;
        opOffset = opOffset_cpy;

        push(globalStack_ptr, returnVal);
        progCount += sizeof(short) + sizeof(byte_t);
        break;
      }

      case OP_NEWARRAY:
      {
        word_t count = pop(globalStack_ptr);
        word_t *array_ptr;
        array_ptr = malloc(count * sizeof(word_t) + 1); //each array stores the count in it's first position
        heap[numOfArrays] = &array_ptr;
        array_ptr[0] = count;

        push(globalStack_ptr, numOfArrays);
        numOfArrays += 1;

        progCount += 1 + sizeof(word_t);
        break;
      }

      case OP_IALOAD:
      {
        word_t array_addr = pop(globalStack_ptr);
        word_t index = pop(globalStack_ptr) + 1;
        word_t *array_ptr = &heap[array_addr];
        if(array_ptr[0] < index)
        {
          isFinished = true;
          break;
        }
        push(globalStack_ptr, array_ptr[index]);
        progCount += 1 + 2 * sizeof(word_t);
        break;
      }

      case OP_IASTORE:
      {
        word_t array_addr = pop(globalStack_ptr);
        word_t index = pop(globalStack_ptr) + 1;
        word_t value = pop(globalStack_ptr);

        word_t *array_ptr = &heap[array_addr];
        if(array_ptr[0] < index)
        {
          isFinished = true;
          break;
        }
        
        array_ptr[index + 1] = value;

        progCount += 1 + 3 * sizeof(word_t);
        break;
      }

      case OP_TAILCALL:
      {
        word_t callerPC, callerLV;
        
        uint16_t indexOfConstant = read_uint16_t(get_text() + progCount + 1); 
        uint32_t startMethodArea = (uint32_t) get_constant(indexOfConstant);
        uint16_t numOfArgs = read_uint16_t(get_text() + startMethodArea); 

        //GETS THE ARGS OUT OF THE STACK
        word_t *arg_storage = malloc(numOfArgs * sizeof(word_t));
        for(int i = 0; i < numOfArgs; i++)
        {
          *(arg_storage + i) = pop(globalStack_ptr);
        }

        word_t linkPtr = globalStack_ptr -> items[lv_addr];
        callerLV = globalStack_ptr -> items[linkPtr+1];
        callerPC = globalStack_ptr -> items[linkPtr];

        globalStack_ptr -> topAddr = (int32_t) lv_addr - 1;

        for(int i = numOfArgs - 1; i >= 0; i--)
        {
          push(globalStack_ptr, (word_t) *(arg_storage + i));
        }
        free(arg_storage);

        //REST OF INVOKE
        uint16_t numOfVars = read_uint16_t(get_text() + sizeof(short) + startMethodArea);
        
        progCount = 4 + startMethodArea; //first instruction is 5 bytes in
        lv_addr = ((uint32_t)globalStack_ptr -> topAddr - numOfArgs + 1); //gets the addres of LV in the stack

        globalStack_ptr -> topAddr += numOfVars; // (int32_t) (opOffset - 1);
        bool loop = false;
        
        while(globalStack_ptr->currMaxSize < globalStack_ptr -> topAddr)
        {
            globalStack_ptr->currMaxSize *= 2;
            loop = true;
        }
        if(loop)
        {
          globalStack_ptr->items = (word_t*) realloc(globalStack_ptr->items, globalStack_ptr->currMaxSize * 2 * sizeof(word_t));
          loop = false;
        }

        push(globalStack_ptr, callerPC);
        globalStack_ptr -> items[lv_addr] = globalStack_ptr -> topAddr;
        push(globalStack_ptr, callerLV);

        break;

      default:
        isFinished = true;
        break;
      }
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

int get_call_stack_size(void) 
{
   return lv_addr;
}


// Checks if reference is a freed heap array. Note that this assumes that 
// 
//bool is_heap_freed(word_t reference) 
//{
   // TODO: implement me
// return 0;
//}
