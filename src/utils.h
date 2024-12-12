#ifndef UTILS_H
#define UTILS_H

#include "typedefs.h"

lli ifindSubsetIn(const int* arr, const size_t n, const size_t start, const int* subset, const size_t s);
void ibuildCombinations(const int* vals, const size_t n, const size_t len, int** out, size_t* outNComb);

double drand01();

#endif // UTILS_H
