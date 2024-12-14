#ifndef DATALOADER_H
#define DATALOADER_H

#include "typedefs.h"

int* loadData_i(const char* file, const char* sep);
void splitTrainTest_i(const int* data, const size_t n, int** trainOut, int** testOut, const double testRatio);

#endif // DATALOADER_H
