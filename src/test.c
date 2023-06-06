#include <stdint.h>  
#include <stdio.h>

int tmain(int argc, char** argv){
  FILE *f = fopen("program1.ijvm", "r");
  if(f != NULL) {
    printf("File opened\n");
    return 0;
  }
  return -1;
}