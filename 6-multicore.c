/*
This assignment explores the operation of out-of-order execution (dynamic scheduling)
and its effectiveness in extracting instruction-level-parallelism. As a fully detailed
simulation of such a core is beyond what could be feasibly done for an assignment,
the model of the processor core described in this assignment is approximate in many ways
(failing to capture many second-order effects), but it does capture the most important
first-order effects of instruction execution.
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
#include <deque.h>
using namespace std;

struct Microop{
    // Each entry of ROB has all following infos
    int32_t sourceReg[3];    //input soiurce physical register
    bool regReady[3];
    int32_t destReg[2];      //output (dest) physical register
    bool isLoad;
    bool isStore;
    uint64_t memAddr;
    int seqNum;
    bool issued;
    int reg1;
    int reg2;
    // Trace When
    int fetchCycle;
    int issueCycle;
    int doneCycle;
    int commitCycle;
    // For Debug
    char macroName[50];
    char microName[50];
    bool misspredictBranch;

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

    // Step #1
    // Avoid WAW & WAR harzards
    int conditionReg = 49;
    // Architecture Register -> Physical Register
    int mapTable[50];
    for(int i = 0; i < 50; i++){mapTable[i] = i;}
    // FIFO free list
    int numPhysReg = 256;
    deque<int> freeList;
    for(int i = 50; i < numPhysReg; i ++) {freeList.push_back(i);}
    // ROB queue
    deque<struct Microop> ROB;
    int robSize;
    // //Q2: Adding Branch Predictor
    // int  predictor_size = 16;
    // int hlength = 16;
    // int branchStallCounter = 0;
    Predictor* predictor = new GSharePredictor(predictor_size, hlength);
    int mispredictPenalty = 5;
    // fprintf(outputFile, "Processing trace...\n");

    // //Q3: Adding Caches
    // int missCachePenalty = 7;
    // Cache* cache =  new Cache(13, 6, 2, 0);
    cache->getInfo();


    while (true) {

        /*
        *   1. Commit
        */
        for(int n = 0; n < N; n ++){
            if(ROB.front() == null) break;
            else if(ROB.front().doneCycle < = currentCycle){
                struct Microop currentOp = ROB.pop_front();
                currentOp.commitCycle = cycle;
                if(currentOp.reg1 != -1)
                    freeList.push_back(currentOp.reg1);
                if(currentOp.reg2 != -1){
                    freeList.push_back(currentOp.reg2);
                }
            }
        }


        /*
        *   2. Issue
        */

        issueCount = 0;
        for(deque<int> ::iterator it  = ROB.begin(); it != ROB.end(); ++it){
            Microop op = (Microop) *it;
            bool isReady = true;
            for(int i = 0; i < 3; i ++){
                int sr = op.sourceReg[i];
                if (sr >= 0 && scoreboard[sr] != 0)
                    isReady = false;
            }

            if(op.memAddress == op2.memAddress){
                int latency = 1;

                if(op.issued == false && isReady){
                    if (op.isLoad){
                        // if(cache->makePrediction();
                        latency = 2;
                    }
                    else
                        latency = 1;

                    op.issued = true;
                    op.issueCycle = cycle;
                    op.doneCycle = cycle + latency;

                    for(int i = 0; i < 2; i ++){
                        int dr = op.destReg[i];
                        scoreBoard[dr] = latency;
                    }

                    issueCount ++;
                    // select N oldest ready instructions.
                    if(issueCount == N)
                        break;
                }
         }
        }

        /*
        *   3. Fetch & Rename
        */

        for(int n = 0; i < N; n ++){

            /*  Experiment 2:   */
            if(fetchReady > 0){
                fetchReady = fetchReady - 1;
                break;
            }
            if(fetchReady == -1){
                break;
            }
            assert(FetchReady == 0);

            if(ROB.size() == robSize)
                break;

            // If no more insns, just break.
            if(lastInsn && ROB.size() == 0)
                break;

            /*
            * 1. Decide whether ready to read next cycle.
            */

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

                // RENAME:
                struct Microop op;
                if(sourceReg[0] >= 0){
                    op.sourceReg[0] = mapTable[(int) sourceRegister];
                }
                if(sourceReg[1] >= 0){
                    op.sourceReg[1] =  mapTable[(int) sourceRegister];
                }
                if(conditionRegister == 'R'){
                    op.sourceReg[2] =mapTable[(int) conditionReg];
                }


                if (conditionRegister =='R')
                {
                    op.reg2 = mapTable[conditionReg];

                if (destinationRegister >= 0){
                    op.reg1 = mapTable[(int)destinationRegister];
                    int newReg = ROB.pop_front();
                    mapTable[(int)destinationRegister] = newReg;
                    op.destReg[0] = newReg;

                    scoreBoard[(int) op.destReg[0]] = -1;
                }

                if (conditionRegister == 'W'){
                    op.destReg[1] = RegFlag;
                    op.reg2 = mapTable[conditionReg];
                    int newReg = ROB.pop_front();
                    mapTable[conditionReg] = newReg;
                    op.reg2 = newReg;
                    SB[(int) op.reg2] = -1;
                }

            /* End of Part1 : Decide whether fetch next insn  */

            // Q2: Branch Prediction:
            // if (branchStallCounter > 0){
            //     break;
            // }



            /*
            * 2. Decide whether the insn is ready
            *    ONLY enter this section when ready, SB = 0
            *    Otherwise wait several cycles
            */
            // sr1Ready = 1;
            // sr2Ready = 1;
            // flagReady = 1;
            // destReady = 1;
            // allReady = 1;
            // if (sourceRegister1 >= 0 && scoreBoard [sourceRegister1] > 0){
            //     sr1Ready = 0;
            // }
            // if (sourceRegister2 >= 0 && scoreBoard [sourceRegister2]  > 0){
            //     sr2Ready = 0;
            // }
            // if (conditionRegister == 'R' || conditionRegister == 'W'){
            //     if(scoreBoard[49] > 0){
            //         flagReady = 0;
            //     }
            // }
            // if(destinationRegister >= 0 &&
            //    scoreBoard[destinationRegister] > 0){
            //     destReady = 0;
            // }
            // // allReady = sr1Ready && sr2Ready  && flagReady && destReady;
            // allReady = sr1Ready && sr2Ready && destReady;
            // // If all are ready:
            // if (allReady){

            //     // @Step 1: Update an insn for its latency
            //     if (destinationRegister >= 0){
            //         if(loadStore == 'L'){
            //             // char hitOrMiss = cache->hitOrMiss(addressForMemoryOp);
            //             // if(hitOrMiss == 'H'){
            //                 latency = 2;
            //                 // printf("hit!\n");
            //             // }
            //             // else
            //                 // printf("miss!\n");
            //                 // latency = 2 +  missCachePenalty;
            //         }

            //         else
            //             latency = 1;
            //         scoreBoard[destinationRegister] = latency;
            //     }

            //     // @Step 2: If insn writes a register set latency
            //     // to the previous insn which it depends
            //     if(conditionRegister == 'W'){
            //         scoreBoard [49] = latency;
            //     }

            //     next = true;

                // @Step 3: (Q2) If insn is conditional branch
                // if (TNnotBranch != '-' && conditionRegister == 'R'){

                //     char bpred = predictor->makePrediction(instructionAddress,
                //                                            TNnotBranch);
                //     if(bpred != TNnotBranch){
                //         branchStallCounter = mispredictPenalty;
                //     }

                //     if(TNnotBranch == 'T'){
                //         break;
                //     }
                // }

            }
            // else
            //     break;
            /* End of Part 2: Decide if it's ready */

        }// End of For loop
        if(loadStore == 'L') op.isLoad=true;
        if(loadStore.charAt(0)=='S') op.isStore=true;
        if(loadStore.charAt(0)=='L' || loadStore.charAt(0)=='S'){
        op.addressForMemoryOp=addressForMemoryOp;
        op.seqNum=totalSeqNum;
        op.FetchCycle=cyc_count;
        if (TNnotBranch != '-' && conditionRegister == 'R') {
            char pred = predictor.makePrediction(instructionAddress, TNnotBranch);
            if (pred != TNnotBranch) {
                fetchReady=-1;
                op.misspredictBranch=true;
            }
        }

        ROB.add(op);
        if (TNnotBranch.charAt(0) == 'T') {
            break;  // a T branch
        }

        }


        /*
        * 3. Advance by one cycle: Decrement 1 for scoreboard & BSCount.
        */
        for (int i=0; i < 256; i++){
            if (scoreBoard[i] != 0){
                scoreBoard[i] = scoreBoard[i] - 1;
            }
        }
        // if(branchStallCounter > 0){
        //     branchStallCounter --;
        // }

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
