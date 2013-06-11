#define __STDC_FORMAT_MACROS
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>

void output(int dist[256]);
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
  int previous=0;
  //Question1
  int mDist[256] = {0};
  //Question2
  int sDist[256] = {0};
  //Question 3
  int bDist[256] ={0};
  int bCumuDist[256] = {0};
  //Qestion 4
  int mixDist[256] = {0};

  //Question 6
  char preFlag= '-';
  int pairCount=0;

  fprintf(outputFile, "Processing trace...\n");


  while (true) {
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
        //Q1
        mDist[previous] = mDist[previous] + 1;
        //Q2

        int size = fallthroughPC - instructionAddress;
        sDist[size] = sDist[size] + 1;
    }
        //Q3
    if (targetAddressTakenBranch != 0){
      int number = 2 + floor( log2(abs(instructionAddress - targetAddressTakenBranch))) ;
        bDist[number] = bDist[number] + 1;
    }
        //Q4
    if ( loadStore == 'L' )
       mixDist[1] = mixDist[1] + 1;
    else if ( loadStore == 'S' )
       mixDist[2] = mixDist[2] + 1;
    else if (targetAddressTakenBranch != 0 && conditionRegister =='-')
       mixDist[3] = mixDist[3] + 1;
    else if (targetAddressTakenBranch != 0 && conditionRegister == 'R')
       mixDist[4] = mixDist[4] + 1;
    else
       mixDist[5] = mixDist[5] + 1;
       //Q5 can be calculated by hand
       //Q6
      if (preFlag == 'R' && targetAddressTakenBranch !=0 ){
        pairCount++;
      }



    previous = microOpCount;
    preFlag = conditionRegister;

  }


  //add the last one
  mDist[previous] = mDist[previous] +1;
  int cumu=0;
  for (int i = 0; i< 256; i ++){
    if (bDist[i]!= 0){
       cumu = cumu + bDist[i];
       bCumuDist[i] = cumu;

    }
  }
  fprintf(outputFile, "Processed %" PRIi64 " trace records.\n", totalMicroops);

  fprintf(outputFile, "Micro-ops: %" PRIi64 "\n", totalMicroops);
  fprintf(outputFile, "Macro-ops: %" PRIi64 "\n", totalMacroops);
  //Q1

  printf("\nQuestion1: The number of micros/macro ops\n");
  output(mDist);
   //Q2
  printf("\nQuestion2: The number for instructions with size(Bytes):\n");
  output(sDist);
  //Q3
  printf("\nQuestion3: The number of branch distance\n");
  output(bCumuDist);
  //Q4
  printf("\nQUestion4: The number of Loads, Stores, Unconditional brances, Conditional branches, and Other\n");
  output(mixDist);

  //Q6
  printf("\n THe number of leagal pairs %d\n", pairCount);
}


void output(int dist[256] ){
  for (int i =0; i<256; i++){
    int number = dist[i];
    if (number !=0){
      printf("The number for value  %d is  %d \n", i, number);
    }
  }
}
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
