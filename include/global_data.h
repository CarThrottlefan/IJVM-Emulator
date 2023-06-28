#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include "ijvm.h"

extern FILE *in;   // use fgetc(in) to get a character from in.
            // This will return EOF if no char is available.
extern FILE *out;  // use for example fprintf(out, "%c", value); to print value to out

extern uint32_t ctNum, txtNum, progCount;
extern int *ctVals, *txtVals;
extern bool isFinished, sameFunc, wide, methodCount;
extern uint32_t opOffset, lv_addr, opOffset_cpy;

extern uint64_t step_num;

#endif // GLOBAL_DATA_H
