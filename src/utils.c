#include "utils.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>

lli ifindSubsetIn(const int* arr, const size_t n, const size_t start, const int* subset, const size_t s) {
    if (!arr || !subset || s > n)
        return -1;

    for (size_t arrI = start; (arrI < n) && (arrI+s); arrI++) {
        if (memcmp(arr+arrI, subset, s*sizeof(int)) == 0)
            return arrI;
    }

    return -1;
}

uint icountSubsetIn(const int* arr, const size_t n, const int* subset, const size_t s) {
    if (!arr || !subset || n < s)
        return 0;

    uint count = 0;
    for (size_t arrI = 0; (arrI < n) && (arrI+s < n); arrI++) {
        if (memcmp(arr+arrI, subset, s * sizeof(int)) == 0)
            count++;
    }

    return count;
}

void combineRecursive(int* comb, const int* vals, const size_t n, const size_t len, size_t depth, int** combs, size_t* combIdx) {
    if (depth == len) {
        for (size_t i = 0; i < len; i++)
            combs[*combIdx][i] = comb[i];
        (*combIdx)++;
        return;
    }

    for (size_t i = 0; i < n; i++) {
        comb[depth] = vals[i];
        combineRecursive(comb, vals, n, len, depth+1, combs, combIdx);
    }
}

void ibuildCombinations(const int* vals, const size_t n, const size_t len, int** out, size_t* outNComb) {
    if (!vals || !out)
        return;

    // Calculate the total number of combinations
    *outNComb = (size_t)pow((double)n, (double)len);
    int* comb = malloc(len * sizeof(int));
    size_t combIdx = 0;
    combineRecursive(comb, vals, n, len, 0, out, &combIdx);

    free(comb);
}

double drand01() {
    return (double)rand() / (double)((unsigned)RAND_MAX + 1);
}
