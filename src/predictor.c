//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Samarth Bhadane";
const char *studentID = "A59019589";
const char *email = "sbhadane@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = {"Static", "Gshare",
                         "Tournament", "Custom"};

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
// TODO: Add your own Branch Predictor data structures here
//

// 2-bit predictions
uint8_t not_taken = 0;
uint8_t weak_not_taken = 1;
uint8_t weak_taken = 2;
uint8_t taken = 3;

// GSHARE
uint8_t *BHT; // Branch History Table
uint8_t history;
int BHT_SIZE;

// TOURNAMENT
uint8_t *GHT;
uint8_t *choicePredictors;
uint8_t *LHT;
uint8_t *PHT;

int GHT_SIZE;
int LHT_SIZE;
int PHT_SIZE;

// PERCEPTRON
int PERCEPTRON_SIZE;
int** weights;
int* historyTable;


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void init_GSHARE()
{
  // printf("Inside GSHARE");
  history = 0;                    // Start with not taken in history
  BHT_SIZE = (1 << ghistoryBits); // 2^{ghistoryBits}
  BHT = (uint8_t *)malloc(BHT_SIZE * sizeof(uint8_t));
  for (int i = 0; i < BHT_SIZE; i++)
  {
    BHT[i] = weak_not_taken; // Initialize all entries with Weak Not Taken
  }
  return;
}

void init_TOURNAMENT()
{
  history = 0;
  LHT_SIZE = (1 << pcIndexBits);  // 2^{pcIndexBits}
  GHT_SIZE = (1 << ghistoryBits); // 2^{ghistoryBits}
  PHT_SIZE = (1 << lhistoryBits); // 2^{lHistoryBits}

  LHT = (uint8_t *)malloc(LHT_SIZE * sizeof(uint8_t));
  for (int i = 0; i < LHT_SIZE; i++)
  {
    LHT[i] = 0;
  }

  GHT = (uint8_t *)malloc(GHT_SIZE * sizeof(uint8_t));
  choicePredictors = (uint8_t *)malloc(GHT_SIZE * sizeof(uint8_t));
  for (int i = 0; i < GHT_SIZE; i++)
  {
    choicePredictors[i] = 0;
    GHT[i] = weak_not_taken; // Initialize all entries with Weak Not Taken
  }

  PHT = (uint8_t *)malloc(PHT_SIZE * sizeof(uint8_t));
  for (int i = 0; i < PHT_SIZE; i++)
  {
    PHT[i] = weak_not_taken; // Initialize all entries with Weak Not Taken
  }

  return;
}

void init_CUSTOM()
{
  ghistoryBits = pcIndexBits = 12;
  int perceptron_size = (1 << pcIndexBits);
  weights = (int**)malloc(perceptron_size * sizeof(int *));
  for(int i = 0; i < perceptron_size; i++){
    weights[i] = (int*)malloc((ghistoryBits + 1) * sizeof(int));
    for(int j = 0; j <= ghistoryBits; j++){
      weights[i][j] = 0;
    }
  }
  historyTable = (int*)malloc(ghistoryBits * sizeof(int));
  for(int i = 0; i < ghistoryBits; i++){
    historyTable[i] = -1;
  }
}

void init_predictor()
{
  //
  // TODO: Initialize Branch Predictor Data Structures
  //
  switch (bpType)
  {
  case STATIC:
    return;
  case GSHARE:
    init_GSHARE();
    return;
  case TOURNAMENT:
    init_TOURNAMENT();
    return;
  case CUSTOM:
    init_CUSTOM();
    return;
  default:
    return;
  }
}

uint8_t
make_prediction_GSHARE(uint32_t pc)
{
  uint32_t index = (pc ^ history) & ((1 << ghistoryBits) - 1);
  if (BHT[index] <= 1)
  {
    return NOTTAKEN;
  }
  else
  {
    return TAKEN;
  }
}

uint8_t
make_prediction_TOURNAMENT(uint32_t pc)
{
  uint8_t global_index = history;
  uint8_t local_index = LHT[pc & (1 << pcIndexBits) - 1];

  uint8_t global_prediction = GHT[global_index] >= 2;
  uint8_t local_prediction = PHT[local_index] >= 2;

  uint8_t chooser_index = global_index;
  uint8_t predictor_choice = choicePredictors[chooser_index] >= 2;

  if (predictor_choice == 1)
  {
    return local_prediction;
  }
  else
  {
    return global_prediction;
  }
}

uint8_t
make_prediction_CUSTOM(uint32_t pc)
{
  int ghr = 0;
  for(int i = 0; i < ghistoryBits; i++){
    ghr = ghr<<1;
    if(historyTable[i] == 1){
      ghr += 1;
    }
  }
  
  int index = (pc ^ ghr) & ((1 << pcIndexBits)-1);
  int y = weights[index][ghistoryBits];
  for(int i = 0; i < ghistoryBits; i++){
    y += weights[index][i]*historyTable[i];
  }
  if(y < 0){
    return NOTTAKEN;
  }

  return TAKEN;
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  // TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType)
  {
  case STATIC:
    return TAKEN;
  case GSHARE:
    return make_prediction_GSHARE(pc);
  case TOURNAMENT:
    return make_prediction_TOURNAMENT(pc);
  case CUSTOM:
    return make_prediction_CUSTOM(pc);
  default:
    break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

void train_predictor_GSHARE(uint32_t pc, uint8_t outcome)
{
  uint32_t index = (pc ^ history) & ((1 << ghistoryBits) - 1);
  if (outcome == TAKEN)
  {
    if (BHT[index] != taken)
    {
      BHT[index] += 1;
    }
  }
  else
  {
    if (BHT[index] != not_taken)
    {
      BHT[index] -= 1;
    }
  }

  history = (history << 1) | outcome;
  return;
}

void train_predictor_TOURNAMENT(uint32_t pc, uint8_t outcome)
{
  uint8_t global_index = history;
  uint8_t local_index = LHT[pc & (1 << pcIndexBits) - 1];

  uint8_t global_prediction = GHT[global_index] >= 2;
  uint8_t local_prediction = PHT[local_index] >= 2;

  uint8_t chooser_index = global_index;
  uint8_t predictor_choice = choicePredictors[chooser_index] >= 2;

  if (outcome == TAKEN)
  {
    if (GHT[global_index] != taken)
    {
      GHT[global_index] += 1;
    }
    if (PHT[local_index] != taken)
    {
      PHT[local_index] += 1;
    }
  }
  else
  {
    if (GHT[global_index] != not_taken)
    {
      GHT[global_index] -= 1;
    }
    if (PHT[local_index] != not_taken)
    {
      PHT[local_index] -= 1;
    }
  }


  LHT[pc & (1 << pcIndexBits) - 1] = ((LHT[pc & (1 << pcIndexBits) - 1] << 1) | outcome);

  if (global_prediction != local_prediction){
    if(outcome == local_prediction){
      choicePredictors[chooser_index] = 1;
    }else{
      choicePredictors[chooser_index] = 0;
    }
  }

  history = (history << 1) | outcome;

  return;
}

void train_predictor_CUSTOM(uint32_t pc, uint8_t outcome)
{
  int result = (outcome == TAKEN)?1:-1;

  int weight_max = (1 << 5) - 1;
  int weight_min = -weight_max - 1;

  int ghr = 0;
  for(int i = 0; i < ghistoryBits; i++){
    ghr = ghr<<1;
    if(historyTable[i] == 1){
      ghr += 1;
    }
  }
  
  int index = (pc ^ ghr) & ((1 << pcIndexBits)-1);
  int y = weights[index][ghistoryBits];
  for(int i = 0; i < ghistoryBits; i++){
    y += weights[index][i]*historyTable[i];
  }

  uint8_t prediction = make_prediction_CUSTOM(pc);

  if(prediction != outcome || (y > weight_min && y < weight_max)){
    weights[index][ghistoryBits] += result;

    for(int i = 0; i < ghistoryBits; i++){
      weights[index][i] += historyTable[i] * result;
    }
  }

  for(int i = ghistoryBits; i > 0; i--){
    historyTable[i] = historyTable[i - 1];
  }historyTable[0] = result;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  // TODO: Implement Predictor training
  //
  switch (bpType)
  {
  case GSHARE:
    train_predictor_GSHARE(pc, outcome);
    return;
  case TOURNAMENT:
    train_predictor_TOURNAMENT(pc, outcome);
    return;
  case CUSTOM:
    train_predictor_CUSTOM(pc, outcome);
  default:
    return;
  }
  return;
}
