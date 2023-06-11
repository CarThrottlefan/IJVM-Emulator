#include <stdio.h>  // for getc, printf
#include <stdlib.h> // malloc, free
#include "ijvm.h"
#include "util.h" // read this file for debug prints, endianness helper functions
#include <string.h>


// see ijvm.h for descriptions of the below functions

FILE *in;   // use fgetc(in) to get a character from in.
            // This will return EOF if no char is available.
FILE *out;  // use for example fprintf(out, "%c", value); to print value to out

uint32_t ctNum, txtNum; //How many bytes there are in Constant and Text respectively
int *ctVals, *txtVals;

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
  uint8_t buf[4]; //FIXME the size for this might be worng
 
  // TODO: implement me
  FILE *f = fopen(binary_path, "rb");
  if(f != NULL) {
    printf("File located at '%s' opened\n", binary_path);
    
    // Magic Number
    fread(buf, sizeof(uint8_t), 4, f); //reads from f 1 byte at a time, the first 4 bytes
    uint32_t magicNum = read_uint32_t(buf); //TODO ask tiziano about this function, what it does
    if(magicNum != 0x1DEADFAD)
    {
      printf("This is not an IJVM file\n");
      return -1;
    }
    
    //Skip past ct origin
    fread(buf, sizeof(uint8_t), 4, f);
    //uint32_t test = read_uint32_t(buf);
    //printf("Is this a new val? 0x%08x\n", test); Used for debugging to check if it skips bytes in read

    //Read constant
    fread(buf, sizeof(uint8_t), 4, f);
    ctNum = read_uint32_t(buf);
    uint32_t cpy = ctNum; 
    ctNum = ctNum / 4; //divided by 4 since each constant is 4 bytes
    
    ctVals = (int *)calloc(ctNum, 4); //allocates a block of 'ctNum' values, each 4 bytes
    fread(ctVals, sizeof(uint16_t), cpy, f); //reads each constant from the block
    for(int i = 0; i < ctNum; i++)
    {
      ctVals[i] = swap_uint32(ctVals[i]);
      //printf("Value: 0x%08x\n", get_constant(i)); //TODO ask what this function needs to do
    }

    //FIXME find out why the buf gets the same data for text
    //Skip past txt origin 
    //memset(buf, 0, sizeof(buf)); - resets buf to be filled with 0's
    fread(buf, sizeof(uint8_t), 4, f);
    uint32_t test = read_uint32_t(buf);
    printf("Is this a new val? 0x%08x\n", test);

    //Read txt
    fread(buf, sizeof(uint8_t), 4 ,f);
    txtNum = read_uint32_t(buf);
    //cpy = txtNum;
    //txtNum = txtNum / 4;
    printf("Txt val is: 0x%08x\n", txtNum);
    
    txtVals = (int *)calloc(txtNum, 4);
    fread(txtVals, sizeof(uint16_t), cpy, f);
    for(int i = 0; i < txtNum; i++)
    {
      txtVals[i] = swap_uint32(txtVals[i]);
      printf("Value: 0x%08x\n", txtVals[i]);
    }

    return 0;
  }
  return -1;
}

void destroy_ijvm(void) 
{
  // TODO: implement me - dealloc all memory, call init
}

byte_t *get_text(void) 
{
  // TODO: implement me
  //return txtVals;
  return 0;
}

unsigned int get_text_size(void) 
{
  // TODO: implement me
  return txtNum;
}

word_t get_constant(int i) 
{
  // TODO: implement me
  return(ctVals[i]);
  //return 0;
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
