/**
Superscalar:

This assignment explores the effectiveness of superscalar execution on program
 performance by extending and integrating your code from the last three homework
  assignments (the pipeline simulator, the branch simulator, and the cache simulator).
   This assignment uses the same traces as previous assignments (see Trace Format document).
As before, you'll write a program that reads in the trace and simulates different processor
 core configurations.
*/
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
#include <iostream>
using namespace std;



// Q2: Branch Prediction: virtual base class
class Predictor{
  public:
  Predictor(int32_t ps){
    predictor_size = ps;
    for (int i=0; i< 1048576; i++){
      HT[i] = 0;
    }
  }
  virtual char makePrediction(uint64_t instructionAddress, char TNotBranch){}
  protected:
  int32_t predictor_size;
  int32_t HT[1048576];
};

// Q2: GShare:
class GSharePredictor:public Predictor{
  public:
  GSharePredictor(int32_t ps, int32_t hl): Predictor(ps){
      hLength = hl;
      hRegister = 0;
  }
  virtual char makePrediction(uint64_t instructionAddress, char TNotBranch){
    int32_t allOne = pow(2, hLength) - 1;
    int32_t mod = (instructionAddress^hRegister) % int32_t(pow(2, predictor_size));
    int32_t counter = HT[mod];

   if (TNotBranch == 'T'){
       hRegister = ((hRegister << 1) & allOne) + 1;
      if     (counter == 3){return 'T';}
      else if(counter == 2){HT[mod] = 3; return 'T';}
      else if(counter == 1){HT[mod] = 2; return 'N';}
      else if(counter == 0){HT[mod] = 1; return 'N';}
    }

    if (TNotBranch == 'N'){
       hRegister = ((hRegister << 1) & allOne);
      if     (counter == 0){return 'N';}
      else if(counter == 1){HT[mod] = 0; return 'N';}
      else if(counter == 2){HT[mod] = 1; return 'T';}
      else if(counter == 3){HT[mod] = 2; return 'T';}
    }
  }
  private:
    int hRegister;
    int hLength;
};

//Q3: Cache:
class twoWayCache{
public:
    twoWayCache(int C, int B){
        for(int i = 0; i < 65536; i ++){
            tag_array0[i] = 0;
            tag_array1[i] = 0;
            LRU[i] = 0;
        }
        cacheBit = C;
        offsetBit = B;
        indexBit = cacheBit - (offsetBit + 1);
        tagBit = 32 - offsetBit - indexBit;

    }
    int32_t getTag(uint64_t address, int32_t num_tag_bits,
                    int32_t num_index_bits, int32_t num_offset_bits){
        int32_t shift = num_offset_bits + num_index_bits;
        return (address >> shift) & ((1 << num_tag_bits) - 1) ;
    }
    int32_t getIndex(uint64_t address, int32_t num_index_bits,
                     int32_t num_offset_bits){
        int mask = (1 << (num_index_bits)) - 1;
        return (address >> num_offset_bits) & mask;
    }

    char hitOrMiss(uint64_t addr){
        int32_t tag = getTag(addr, tagBit, indexBit, offsetBit);
        int32_t index = getIndex(addr, indexBit, offsetBit);
        // Hit! Equals to either tag
        if(tag == tag_array0[index]){
            hit ++;
            LRU[index] = 1;
            return 'H';
        }
        else if(tag == tag_array1[index]){
            hit ++;
            LRU[index] = 0;
            return 'H';
        }
        // Miss!
        else{
            miss ++;
            if(LRU[index] == 0){
                tag_array0[index] = tag;
                LRU[index] = 1;
            }
            else if(LRU[index] == 1){
                tag_array1[index] = tag;
                LRU[index] = 0;
            }
            return 'M';
        }
    }

    void getInfo(){
        printf("cacheBit: %d, offsetBit: %d, indexBit: %d, tagBit: %d\n",
                cacheBit, offsetBit, indexBit, tagBit);
    }

private:
    int32_t cacheBit;
    int32_t offsetBit;
    int32_t indexBit;
    int32_t tagBit;
    int32_t tag_array0[65536];
    int32_t tag_array1[65536];
    int32_t LRU [65536];
    int hit;
    int miss;
};



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

    //Q1: Adding Pipeline Simulator
    bool next = 1;
    bool sr1Ready = 0;
    bool sr2Ready = 0;
    bool flagReady = 0;
    bool destReady = 0;
    bool allReady = 0;
    int scoreBoard[256] = {0};
    int cycle = 0;
    int N = 6;  // N = 1 ~ 6
    int latency = 1;
    bool lastInsn = 0;

    //Q2: Adding Branch Predictor
    int  predictor_size = 16;
    int hlength = 16;
    int branchStallCounter = 0;
    Predictor* predictor = new GSharePredictor(predictor_size, hlength);
    int mispredictPenalty = 5;
    fprintf(outputFile, "Processing trace...\n");

    //Q3: Adding Caches
    int missCachePenalty = 7;
    twoWayCache* cache =  new twoWayCache(13, 6);
    cache->getInfo();


    while (true) {

        if(lastInsn)
            break;

        for(int n = 0; n < N; n ++){

            /*
            * 1. Decide whether ready to read next cycle.
            */
            if (next){

                // read next micro-op from trace
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
                  lastInsn = 1;
                  break;
                }

                if (result != 14) {
                  fprintf(stderr, "Error parsing trace at line%"
                    PRIi64 "\n", totalMicroops);
                  abort();
                }

                // For each micro-op
                totalMicroops++;

                // For each macro-op:
                if (microOpCount == 1) {
                  totalMacroops++;
                }

                // Set next to false after insn read
                next = false;


            }
            /* End of Part1 : Decide whether fetch next insn  */

            // Q2: Branch Prediction:
            if (branchStallCounter > 0){
                break;
            }

            /*
            * 2. Decide whether the insn is ready
            *    ONLY enter this section when ready, SB = 0
            *    Otherwise wait several cycles
            */
            sr1Ready = 1;
            sr2Ready = 1;
            flagReady = 1;
            destReady = 1;
            allReady = 1;
            if (sourceRegister1 >= 0 && scoreBoard [sourceRegister1] > 0){
                sr1Ready = 0;
            }
            if (sourceRegister2 >= 0 && scoreBoard [sourceRegister2]  > 0){
                sr2Ready = 0;
            }
            if (conditionRegister == 'R' || conditionRegister == 'W'){
                if(scoreBoard[49] > 0){
                    flagReady = 0;
                }
            }
            if(destinationRegister >= 0 &&
               scoreBoard[destinationRegister] > 0){
                destReady = 0;
            }
            allReady = sr1Ready && sr2Ready  && flagReady && destReady;

            // If all are ready:
            if (allReady){

                // @Step 1: Update an insn for its latency
                if (destinationRegister >= 0){
                    if(loadStore == 'L'){
                        char hitOrMiss = cache->hitOrMiss(addressForMemoryOp);
                        if(hitOrMiss == 'H'){
                            latency = 2;
                            // printf("hit!\n");
                        }
                        else
                            // printf("miss!\n");
                            latency = 2 +  missCachePenalty;
                    }

                    else
                        latency = 1;
                    scoreBoard[destinationRegister] = latency;
                }

                // @Step 2: If insn writes a register set latency
                // to the previous insn which it depends
                if(conditionRegister == 'W'){
                    scoreBoard [49] = latency;
                }

                next = true;

                // @Step 3: (Q2) If insn is conditional branch
                if (TNnotBranch != '-' && conditionRegister == 'R'){

                    char bpred = predictor->makePrediction(instructionAddress,
                                                           TNnotBranch);
                    if(bpred != TNnotBranch){
                        branchStallCounter = mispredictPenalty;
                    }

                    if(TNnotBranch == 'T'){
                        break;
                    }
                }

            }
            else
                break;
            /* End of Part 2: Decide if it's ready */

        }// End of For loop

        /*
        * 3. Advance by one cycle: Decrement 1 for scoreboard & BSCount.
        */
        for (int i=0; i < 256; i++){
            if (scoreBoard[i] != 0){
                scoreBoard[i] = scoreBoard[i] - 1;
            }
        }
        if(branchStallCounter > 0){
            branchStallCounter --;
        }

        //Add cycle count.
        cycle++;



    }// End of WHILE



    fprintf(outputFile, "Processed %" PRIi64 " trace records.\n", totalMicroops);
    fprintf(outputFile, "Micro-ops: %" PRIi64 "\n", totalMicroops);
    fprintf(outputFile, "Macro-ops: %" PRIi64 "\n", totalMacroops);

    // Print # of cycles
    printf("2: The number of cycles for N = %d is  %d \n", N, cycle);
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
