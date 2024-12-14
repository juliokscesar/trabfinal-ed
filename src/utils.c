#include "utils.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "logging.h"

lli findSubsetIn_i(const int* arr, const size_t n, const size_t start, const int* subset, const size_t s) {
    if (!arr || !subset || s > n)
        return -1;

    for (size_t arrI = start; (arrI < n) && (arrI+s); arrI++) {
        if (memcmp(arr+arrI, subset, s*sizeof(int)) == 0)
            return arrI;
    }

    return -1;
}

uint countSubsetIn_i(const int* arr, const size_t n, const int* subset, const size_t s) {
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

void buildCombinations_i(const int* vals, const size_t n, const size_t len, int** out, size_t* outNComb) {
    if (!vals || !out)
        return;

    // Calculate the total number of combinations
    *outNComb = (size_t)pow((double)n, (double)len);
    int* comb = malloc(len * sizeof(int));
    size_t combIdx = 0;
    combineRecursive(comb, vals, n, len, 0, out, &combIdx);

    free(comb);
}

void printArr_i(const int* arr, const size_t n) {
    if (!arr)
        return;
    for (size_t i = 0; i < n; i++) {
        printf("%d%c ", arr[i], ((i < (n-1)) ? ',' : '\0'));
    }
    putchar('\n');
}

double rand01_d() {
    return (double)rand() / (double)((unsigned)RAND_MAX + 1);
}

int* loadData_i(const char* file, size_t* outN) {
    if (!file)
        return NULL;

    FILE* df = fopen(file, "r");
    if (!df) {
        LOG_ERROR("Unable to open data file");
        return NULL;
    }

    // pre-allocate at least 20 ints
    const size_t BUCKET = 20;
    size_t nBuckets = 1;
    int* data = malloc(BUCKET*sizeof(int));
    size_t n = 0;

    while (!feof(df)) {
        // scan integer to data
        fscanf(df, "%d", &data[n]);
        n++;

        // check if we need to increase bucket
        if (n > nBuckets*BUCKET) {
            nBuckets++;
            int* newData = realloc(data, nBuckets * BUCKET * sizeof(int));
            if (!newData) {
                LOG_ERROR("Failed to allocate more space for loading data.");
                break;
            }
            data = newData;
        }
    }

    fclose(df);
    *outN = n;
    return data;
}

void splitTrainTest_i(const int* data, const size_t n, int** trainOut, int** testOut, size_t* trainSizeOut, size_t* testSizeOut, const double testRatio) {
    if (!data || !trainOut || !testOut)
        return;

    const size_t testSize = testRatio * n;
    *testOut = malloc(sizeof(int) * testSize);
    if (!(*testOut)) {
        LOG_ERROR("Failed to allocate memory for testOut");
        return;
    }

    const size_t trainSize = n - testSize;
    *trainOut = malloc(sizeof(int) * trainSize);
    if (!(*trainOut)) {
        LOG_ERROR("Failed to allocate memory for trainOut");
        free(*testOut);
        *testOut = NULL;
        return;
    }

    // Fill training first
    memcpy(*trainOut, data, sizeof(int) * trainSize);
    // Fill test
    memcpy(*testOut, data + trainSize, sizeof(int) * testSize);

    *trainSizeOut = trainSize;
    *testSizeOut = testSize;
}

double calcAccuracy(const int* truth, const int* predicted, const size_t n) {
    if (!truth || !predicted)
        return -1.0;
    if (n == 0)
        return 0.0;

    double correct = 0.0;
    for (size_t i = 0; i < n; i++)
        correct += (double)(truth[i] == predicted[i]);
    return correct / (double)n;
}
