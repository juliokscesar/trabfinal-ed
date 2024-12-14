#ifndef UTILS_H
#define UTILS_H

#include "typedefs.h"

lli findSubsetIn_i(const int* arr, const size_t n, const size_t start, const int* subset, const size_t s);
uint countSubsetIn_i(const int* arr, const size_t n, const int* subset, const size_t s);
void buildCombinations_i(const int* vals, const size_t n, const size_t len, int** out, size_t* outNComb);

void printArr_i(const int* arr, const size_t n);
double rand01_d();

int* loadData_i(const char* file, size_t* outN);
void saveData_i(const int* data, size_t n, const char* file);
void splitTrainTest_i(const int* data, const size_t n, int** trainOut, int** testOut, size_t* trainSizeOut, size_t* testSizeOut, const double testRatio);
void splitTrainValTest_i(const int* data, const size_t n, int** trainOut, int** valOut, int** testOut, size_t* trainSizeOut, size_t* valSizeOut, size_t* testSizeOut, const double valRatio, const double testRatio);

double calcAccuracy(const int* truth, const int* predicted, const size_t n);

#endif // UTILS_H
