#define __STDC_FORMAT_MACROS
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

void simulate(FILE* inputFile, FILE* outputFile)
{
  // See the documentation to understand what these variables mean.
  int32_t microOpCount;
  uint64_t instructionAddress;
  int32_t sourceRegister1;
  int32_t sourceRegister2;
  int32_t destinationRegister;
  char conditionRegister;
  char TNnotBranch;
  char loadStore;
  int64_t immediate;
  uint64_t addressForMemoryOp;
  uint64_t fallthroughPC;
  uint64_t targetAddressTakenBranch;
  char macroOperation[12];
  char microOperation[23];

  int64_t totalMicroops = 0;
  int64_t totalMacroops = 0;
  bool next = 1;
  bool sr1Ready = 0;
  bool sr2Ready = 0;
  int scoreBoard[256];
  int cycle = 0;
  // Debug: latency = 1 ~ 15
  int latency = 1;
  fprintf(outputFile, "Processing trace...\n");
  
  while (true) {
    if (next){
    int result = fscanf(inputFile, 
                        "%" SCNi32
                        "%" SCNx64 
                        "%" SCNi32
                        "%" SCNi32
                        "%" SCNi32
                        " %c"
                        " %c"
                        " %c"
                        "%" SCNi64
                        "%" SCNx64
                        "%" SCNx64
                        "%" SCNx64
                        "%11s"
                        "%22s",
                        &microOpCount,
                        &instructionAddress,
                        &sourceRegister1,
                        &sourceRegister2,
                        &destinationRegister,
                        &conditionRegister,
                        &TNnotBranch,
                        &loadStore,
                        &immediate,
                        &addressForMemoryOp,
                        &fallthroughPC,
                        &targetAddressTakenBranch,
                        macroOperation,
                        microOperation);
                        
    if (result == EOF) {
      break;
    }

    if (result != 14) {
      fprintf(stderr, "Error parsing trace at line %" PRIi64 "\n", totalMicroops);
      abort();
    }

    // For each micro-op
    totalMicroops++;

    // For each macro-op:
    if (microOpCount == 1) {
      totalMacroops++;
    }
    next = false;
    }

  /*
  next_instruction = true
  while (true):
      if (next_instruction):
          read next micro-op from trace
          next_instruction = false
  
      if instruction is "ready" based on scoreboard entries for valid input and output registers:
          if the instruction writes a register:
              "execute" the instruction by recording when its register will be ready
          next_instruction = true

         advance by one cycle by decrementing non-zero entries in the scoreboard
         increment total cycles counter
  */
  
  if (sourceRegister1 == -1 || scoreBoard [sourceRegister1]== 0){
    sr1Ready = 1;
  }
  

  if (sourceRegister2 == -1 || scoreBoard [sourceRegister2]== 0){
    sr2Ready = 1;
  }

  if (sr1Ready == 1 && sr2Ready == 1){
    
    if (destinationRegister == -1){
      next = true;
    }
    else if(scoreBoard [destinationRegister] == 0){

      if(loadStore == 'L'){
          scoreBoard [destinationRegister] = latency;
      }
      else{
          scoreBoard [destinationRegister] = 1;
     }
     //Now we can go.
     next = true;
    }
  }

   
   //Decrease scores in scoreBoard
   for (int i=0; i < 256; i++){
     if (scoreBoard[i] != 0){
       scoreBoard[i] = scoreBoard[i] - 1;
     }
   }
   //Increase the cycle count  
   cycle++;
     
  }
  
  fprintf(outputFile, "Processed %" PRIi64 " trace records.\n", totalMicroops);

  fprintf(outputFile, "Micro-ops: %" PRIi64 "\n", totalMicroops);
  fprintf(outputFile, "Macro-ops: %" PRIi64 "\n", totalMacroops);
  printf("The number of cycles for latency  %d is  %d \n", cycle, latency);
}

/*
void output(int dist[256] ){
  for (int i =0; i<256; i++){
    int number = dist[i];
    if (number !=0){
      printf("The number for value  %d is  %d \n", i, number);
    }
  }
}
*/

int main(int argc, char *argv[]) 
{
  FILE *inputFile = stdin;
  FILE *outputFile = stdout;
  
  if (argc >= 2) {
    inputFile = fopen(argv[1], "r");
    assert(inputFile != NULL);
  }
  if (argc >= 3) {
    outputFile = fopen(argv[2], "w");
    assert(outputFile != NULL);
  }
  
  simulate(inputFile, outputFile);
  return 0;
}
