/*
Branch Prediction:
In this assignment you will explore the effectiveness of branch direction prediction (taken vs not taken)
 on an actual program. Your task is to use the trace format (see Trace Format document) to evaluate the
 effectiveness of a few simple branch prediction schemes.
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
//#include "BimodalPredictor.h"
//#include "GSharePredictor.h"
//#include "TournamentPredictor.h"

//Virtual Base Class
class Predictor{
  public:
  Predictor(int32_t ps){
    predictor_size = ps;
    for (int i=0; i< 1048576; i++){
      HT[i] = 0;
    }
  }
  virtual char makePrediction(uint64_t instructionAddress, char TNotBranch){}
  virtual void print(FILE *debugOutputFile){}
  virtual string getConfig(){}
  protected:
  int32_t predictor_size;
  int32_t HT[1048576];
};

//Q1-Q2: Bimodal
class BimodalPredictor:public Predictor{
  public:
  BimodalPredictor(int32_t ps): Predictor(ps){}
  virtual char makePrediction(uint64_t instructionAddress, char TNotBranch){

    int mod = instructionAddress % int32_t(pow(2, predictor_size));
    int32_t counter = HT[mod];
    if (TNotBranch == 'T'){
      if     (counter == 3){return 'T';}
      else if(counter == 2){HT[mod] = 3; return 'T';}
      else if(counter == 1){HT[mod] = 2; return 'N';}
      else if(counter == 0){HT[mod] = 1; return 'N';}
    }
    if (TNotBranch == 'N'){
      if     (counter == 0){return 'N';}
      else if(counter == 1){HT[mod] = 0; return 'N';}
      else if(counter == 2){HT[mod] = 1; return 'T';}
      else if(counter == 3){HT[mod] = 2; return 'T';}
    }
  }
  virtual void print(FILE *debugOutputFile){
    cout<< "debuging ...";
  }
  virtual string getConfig(){
    return "Hi";
  }
};

//GShare:
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

  //return 'T';
  }
  virtual void print(FILE *debugOutputFile){
    cout<< "debuging ...";
  }
  virtual string getConfig(){
    return "Hi";
  }
  private:
    int hRegister;
    int hLength;
};


//Tournament
class TournamentPredictor:public Predictor{
  public:
  TournamentPredictor(int32_t ps, BimodalPredictor *bp, GSharePredictor *gs):
                      Predictor(ps), bPredictor(bp), gPredictor(gs){}
  virtual char makePrediction(uint64_t instructionAddress, char TNotBranch){

    int mod = instructionAddress % int32_t(pow(2, predictor_size));
    int32_t choice = this ->HT[mod];
    char bpred = bPredictor -> makePrediction(instructionAddress, TNotBranch);
    char gpred = gPredictor -> makePrediction(instructionAddress, TNotBranch);
    char tpred;

    /*
    * First choose which predictor tournament table currently believes in.
    */
    if (choice == 0 || choice == 1){tpred = bpred; }
    if (choice == 2 || choice == 3){tpred = gpred; }

    /*
    * Then update tournament table.
    * If Ghare and Bimodal predict the same, don't do anything.
    */

    if (bpred != gpred){

      //If GShare predicts right, add one to the tournament table.
      if (TNotBranch == gpred){
        if     (choice == 2){HT[mod] = 3;}
        else if(choice == 1){HT[mod] = 2;}
        else if(choice == 0){HT[mod] = 1;}
      }
      //If GShare predicts right, minus one to the tournament table.
      if (TNotBranch == bpred){
        if     (choice == 1){HT[mod] = 0;}
        else if(choice == 2){HT[mod] = 1;}
        else if(choice == 3){HT[mod] = 2;}
      }

    }
    //Return Tournament's prediction.
    //if (tpred==TNotBranch){cout<<TNotBranch<<"  "<<tpred<< "  correct"<<endl;}
    //else{cout<<TNotBranch<<"  "<<tpred<< "  incorrect"<<endl;}
    return tpred;
  }

  virtual void print(FILE *debugOutputFile){
    cout<< "debuging ...";
  }
  virtual string getConfig(){
    return "Hi";
  }
  private:
    BimodalPredictor *bPredictor;
    GSharePredictor *gPredictor;
};




void simulate(int32_t predictor_size, FILE* inputFile, FILE* outputFile, FILE* debugOutputFile, FILE* debugInputFile)
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
  int64_t totalCondBranches = 0;

  int correct = 0;
  int incorrect = 0;

  // Initialization - Bimodal:
  // predictor_size = 3;
  // Predictor* predictor = new BimodalPredictor(predictor_size);

  //Initialization - GShare:
  predictor_size = 10;
  int hlength = 14;
  cout<<endl<<"[predictor_size] "<<predictor_size<<",  [history_length] "<<hlength<<endl;  Predictor* predictor = new GSharePredictor(predictor_size, hlength);
  /*
   //Initialization - Tournament:
  int32_t n = 20;
  int32_t tsize = n - 2;
  int32_t bsize = n - 2;
  int32_t gsize = n - 1;
  cout<<"gsize:  "<<gsize<<",   bsize =csize ="<<bsize<<endl;
  Predictor* predictor = new TournamentPredictor(tsize, new BimodalPredictor(bsize), new GSharePredictor(gsize, gsize));
  */
  fprintf(stderr, "Processing trace...\n");

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

    if (TNnotBranch == '-') {
      continue;  // Not a branch
    }

    if (conditionRegister != 'R') {
      continue;  // Not a conditional branch
    }

    totalCondBranches++;

    if (debugOutputFile) { // debug output trace
      predictor->print(debugOutputFile);
    }

    char pred = predictor->makePrediction(instructionAddress, TNnotBranch);

    if (pred == TNnotBranch) {
      correct++;
    } else {
      incorrect++;
    }

    if (debugOutputFile) { // debug output trace
      fprintf(debugOutputFile, "| %" PRIx64 "  %c | %c  %s%d\n",
              instructionAddress,
              TNnotBranch,
              pred,
              (pred == TNnotBranch) ? "correct    " : "incorrect  ",
              incorrect);
    }

    if (debugInputFile) {
      int expected_incorrect;
      fscanf(debugInputFile, "%*[^|] %*s %*s %*s %*s %*s %*s %d",  // Read many fields, throwing away all but the last
             &expected_incorrect);

      if (incorrect != expected_incorrect) {
        fprintf(stderr, "Mismatched mis-prediction count: %d vs %d\n", incorrect, expected_incorrect);
        abort();
      }
    }

  }

  fprintf(stderr, "Processed %" PRIi64 " trace records.\n", totalMicroops);

  std::string s = predictor->getConfig();
  fprintf(outputFile, "%45s %11i %11i %8.2f%%\n",
          s.c_str(), correct, incorrect, 100.0*incorrect/(correct+incorrect));


}

int main(int argc, char *argv[])
{
  int32_t predictor_size = 1;
  FILE *inputFile = stdin;
  FILE *outputFile = stdout;
  FILE *debugInputFile = NULL;
  FILE *debugOutputFile = NULL;
  if (argc >= 2) {
    predictor_size = atoi(argv[1]);
  }

  if (argc >= 3) {
    debugOutputFile = fopen(argv[2], "w");
    assert(debugOutputFile != NULL);
  }

  if (argc >= 4) {
    debugInputFile = fopen(argv[3], "r");
    assert(debugInputFile != NULL);
  }
  simulate(predictor_size, inputFile, outputFile, debugOutputFile, debugInputFile);
  return 0;
}
