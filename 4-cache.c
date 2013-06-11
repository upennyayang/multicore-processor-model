/*
Cache:
In this assignment you will explore the effectiveness of caches and the impact of
different cache configurations. Your task will be use the program trace format used in the previous assignments
(see Trace Format document) to evaluate the impact of cache size, associativity, block size, etc.
To do this, you'll write a program that reads in the trace and simulates different cache configurations.
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

void output(int dist[256]);
int32_t getTag(uint64_t,int32_t, int32_t, int32_t);
int32_t getIndex(uint64_t, int32_t, int32_t);

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

  int32_t way = 2;
  int32_t cacheBit = 15;
  int32_t blockBit = 9;
  int32_t offsetBit = blockBit;
  int32_t indexBit;
  if (way == 1){indexBit = cacheBit - blockBit;}
  if (way == 2){indexBit = cacheBit - (blockBit + 1);}
  int32_t tagBit = 32 - offsetBit - indexBit;


  int32_t hit = 0;
  int32_t miss = 0;
  int32_t reference = 0;
  int32_t missPredict = 0;


  // 16 bit index has max number of index. pow(2, 16) =  65536
  int32_t tag_array[65536] = {0};
  // Q5:
  int32_t tag_array0[65536] = {0};
  int32_t tag_array1[65536] = {0};
  int32_t LRU [65536] ={0};
  int32_t dirty0[65536] = {0};
  int32_t dirty1[65536] = {0};
  int32_t store = 0;
  int32_t dirtyEviction = 0;
  //Q8
  int32_t wayPredictor[4] = {1};
  int32_t n = 0;
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
    }
        //Q4
    if ( loadStore == 'L' || loadStore == 'S'){

        reference ++;
        int32_t tag = getTag(addressForMemoryOp, tagBit, indexBit, offsetBit);
        //Use index to locate cache line first. Then check tag value: hit/miss
        int32_t index = getIndex(addressForMemoryOp,  indexBit, offsetBit);
        //Q4 way = 1
        if(way == 1 ){
            int tag_value = tag_array[index];

            if (tag_value == tag){
                hit ++;
              //  printf("hit!\t");
            }else if(tag_value != tag){
                miss ++;
                tag_array[index] = tag;
               // printf("miss!\t");
            }
            //printf("[Memory] %x    [Tag] %d     [Index] %d \n", (unsigned int)addressForMemoryOp, tag,index);
        }

        else if(way == 2){
            //Q8
            int32_t predictBit = (addressForMemoryOp >> offsetBit) & ((1 << n) - 1);
            int32_t tag_value0 = tag_array0[index];
            int32_t tag_value1 = tag_array1[index];
            if(loadStore == 'S'){
                store ++;
            }
            /*
            * HIT!!!
            */
            //Match way 0
            if(tag == tag_value0){
                if(wayPredictor[predictBit] == 1){
                    missPredict ++;
                    wayPredictor[predictBit] = 0;
                }
                hit ++;
                LRU[index] = 1;
                if(loadStore == 'S'){
                    dirty0[index] = 1;
                }
                //printf("hit!\t");
            }
            //Match way 1
            else if(tag == tag_value1){
                if(wayPredictor[predictBit] == 0){
                    missPredict ++;
                    wayPredictor[predictBit] = 1;
                }

                hit ++;
                LRU[index] = 0;
                if(loadStore == 'S'){
                    dirty1[index] = 1;
                }
                //printf("hit!\t");
            }
            /*
            * MISS!!! Doesn't match both way 0 & way 1
            */
            else {
                miss ++;

                //LRU = 1, evict way 1
                if(LRU[index] == 1){
                    wayPredictor[predictBit] = 1;
                    tag_array1[index] = tag;
                    LRU[index] = 0;
                    //Q6: Load, evict and check.
                    if (dirty1[index] == 1){
                        //Write back to memory.
                        dirtyEviction ++;
                    }
                    dirty1[index] = 1;
                    if (loadStore == 'L'){
                        //Bring Memory to Cache, clean
                        dirty1[index] = 0;
                    }
                }
                //LRU = 0, evict way 0
                else{
                    wayPredictor[predictBit] = 0;
                    tag_array0[index] = tag;
                    LRU[index] = 1;
                    if(dirty0[index] == 1){
                        //Write back to memory.
                        dirtyEviction ++;
                    }
                    dirty0[index] = 1;
                    if(loadStore == 'L'){
                        //Bring Memory to Cache, clean
                        dirty0[index] = 0;
                    }
                }
            }
           //printf("[Memory] %x    [Tag] %d     [Index] %d \n", (unsigned int)addressForMemoryOp, tag,index);
        }


    }

  }

  fprintf(outputFile, "Processed %" PRIi64 " trace records.\n", totalMicroops);

  fprintf(outputFile, "Micro-ops: %" PRIi64 "\n", totalMicroops);
  fprintf(outputFile, "Macro-ops: %" PRIi64 "\n", totalMacroops);

  //Q4, Q5, Q7
  float mRatio = (100 * (float)miss) / (miss + hit);
  printf("Cache Bit: %d, Block Bit: %d, Miss Rate:%.3f \n", cacheBit, blockBit, mRatio);

  //Q6
  int32_t blockSize = pow(2, blockBit);
  float wtTraffic = (float) (miss * blockBit + store * 4) / reference;
  float wbTraffic = (float) (miss * blockBit + dirtyEviction * blockBit) / reference;
  printf("Cache Bit: %d, reference: %d, store: %d, dirtyEviction: %d, write-through: %.3f, write-back: %.3f\n",
          cacheBit, reference, store, dirtyEviction, wtTraffic, wbTraffic);

  //Q8
  float ratio = (float) (missPredict) / hit;
  printf("n:  %d, ratio:   %.3f\n", hit, n, ratio);


}


int32_t getTag(uint64_t address, int32_t num_tag_bits, int32_t num_index_bits, int32_t num_offset_bits){
    int32_t shift = num_offset_bits + num_index_bits;
    return (address >> shift) & ((1 << num_tag_bits) - 1) ;
}
int32_t getIndex(uint64_t address, int32_t num_index_bits, int32_t num_offset_bits){
    int mask = (1 << (num_index_bits)) - 1;
    return (address >> num_offset_bits) & mask;
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
