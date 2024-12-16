#include "utils.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

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
        printf("%d%s", arr[i], ((i < (n-1)) ? ", " : ""));
    }
    putchar('\n');
}

void printArr_d(const double* arr, const size_t n) {
    if (!arr)
        return;
    for (size_t i = 0; i < n; i++) {
        printf("%lf%s", arr[i], ((i < (n-1)) ? ", " : ""));
    }
    putchar('\n');
}

double rand01_d() {
    return (double)rand() / (double)((unsigned)RAND_MAX + 1);
}

int _cmpAsc(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

void findDistinct_i(const int* data, const size_t n, int** out, size_t* outSize) {
    if (!data || !out || !outSize || n == 0)
        return;

    int* copy = malloc(sizeof(int) * n);
    if (!copy) {
        LOG_ERROR("malloc failed for allocating data copy, unable to sort in findDistinct");
        return;
    }
    memcpy(copy, data, sizeof(int) * n);

    // First sort array so all changes become consecutive
    qsort(copy, n, sizeof(int), _cmpAsc);

    *out = malloc(sizeof(int)*n);
    if (!(*out)) {
        LOG_ERROR("Unable to allocate memory for out array in findDistinct");
        free(copy);
        return;
    }

    size_t outI = 0;
    for (size_t dataI = 0; dataI < n; dataI++) {
        if (dataI == 0 || copy[dataI] != copy[dataI - 1]) {
            (*out)[outI] = copy[dataI];
            outI++;
        }
    }
    *outSize = outI;

    free(copy);

    // If outSize is less than n, resize the distinct array to save memory
    if (*outSize < n) {
        int* temp = realloc(*out, (*outSize) * sizeof(int));
        if (!temp) {
            LOG_WARNING("Tried to reallocate out array with less memory, but failed. Returning as it is");
            return;
        }
        *out = temp;
    }
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

    char line[256];
    while (!feof(df)) {
        // scan integer to data
        char* res = fgets(line, sizeof(line), df);
        if (!res)
            break;

        const long val = strtol(line, NULL, 10);
        if (errno == ERANGE || val > INT_MAX || val < INT_MIN)
            continue;

        data[n] = (int)val;
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
    if (testRatio > 1.0)
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

void splitTrainValTest_i(const int* data, const size_t n, int** trainOut, int** valOut, int** testOut,
    size_t* trainSizeOut, size_t* valSizeOut, size_t* testSizeOut, const double valRatio, const double testRatio) {
    if (!data || !trainOut || !testOut)
        return;

    if (fabs(1.0 - (valRatio + testRatio)) < 1e-4)
        return;

    *valSizeOut = n * valRatio;
    *testSizeOut = n * testRatio;
    *trainSizeOut = n - *valSizeOut - *testSizeOut;

    *trainOut = malloc(sizeof(int) * *trainSizeOut);
    memcpy(*trainOut, data, sizeof(int) * *trainSizeOut);

    *valOut = malloc(sizeof(int) * *valSizeOut);
    memcpy(*valOut, data+(*trainSizeOut), sizeof(int) * *valSizeOut);

    *testOut = malloc(sizeof(int) * *testSizeOut);
    memcpy(*testOut, data+(*trainSizeOut)+(*valSizeOut), sizeof(int) * *testSizeOut);
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

double** confusionMatrix(const int* truth, const int* predicted, const size_t n, size_t* outRows, size_t* outCols) {
    if (!truth || !predicted)
        return NULL;

    int *uniqueTrue, *uniquePred;
    size_t unqTrueSz, unqPredSz;
    findDistinct_i(truth, n, &uniqueTrue, &unqTrueSz);
    findDistinct_i(predicted, n, &uniquePred, &unqPredSz);
    if (!uniqueTrue || !uniquePred) {
        LOG_ERROR("Unable to get unique values from truth or predicted values");
        return NULL;
    }
    if (unqTrueSz != unqPredSz)
        LOG_WARNING("Truth and Predicted arrays have different unique values");

    // Initialize confusion matrix
    *outRows = unqTrueSz;
    *outCols = unqPredSz;
    double** cm = malloc(sizeof(double*) * *outRows);
    if (!cm) {
        LOG_ERROR("malloc failed for confusion matrix");
        free(uniqueTrue); free(uniquePred);
        return NULL;
    }
    for (size_t i = 0; i < *outRows; i++) {
        cm[i] = calloc(*outCols, sizeof(double));
        if (!cm[i]) {
            LOG_ERROR("calloc failed for initializing confusion matrix columns");
            for (size_t j = 0; j < i; j++)
                free(cm[j]);
            free(cm); free(uniqueTrue); free(uniquePred);
            return NULL;
        }
    }

    // Now build confusion matrix
    for (size_t dataI = 0; dataI < n; dataI++) {
        // id value based on unique values. unique array is sorted so use bsearch
        int* inTrue = bsearch(&truth[dataI], uniqueTrue, unqTrueSz, sizeof(int), _cmpAsc);
        if (!inTrue) {
            LOG_ERROR("bsearch failed for finding value in uniqueTrue:");
            printf("ID: %lu, truth[dataI]=%d\n", dataI, truth[dataI]);
            continue;
        }
        const size_t trueIdx = inTrue - uniqueTrue;

        int* inPred = bsearch(&predicted[dataI], uniquePred, unqPredSz, sizeof(int), _cmpAsc);
        if (!inPred) {
            LOG_ERROR("bsearch failed for finding value in uniquePred:");
            printf("ID: %lu, truth[dataI]=%d\n", dataI, predicted[dataI]);
            continue;
        }
        const size_t predIdx = inPred - uniquePred;
        cm[trueIdx][predIdx]++;
    }

    free(uniqueTrue);
    free(uniquePred);
    return cm;
}

void showConfusionMatrix(const int* truth, const int* predicted, const size_t n) {
    if (!truth || !predicted)
        return;

    size_t cmRows, cmCols;
    double** cm = confusionMatrix(truth, predicted, n, &cmRows, &cmCols);
    if (!cm) {
        LOG_ERROR("Unable to get confusion matrix in showConfusionMatrix");
        return;
    }

    int *uniqueTrue, *uniquePred;
    size_t unqTrueSz, unqPredSz;
    findDistinct_i(truth, n, &uniqueTrue, &unqTrueSz);
    findDistinct_i(predicted, n, &uniquePred, &unqPredSz);
    if (!uniqueTrue || !uniquePred) {
        LOG_ERROR("Unable to get unique values from truth or predicted values");
        return;
    }
    if (unqTrueSz != unqPredSz)
        LOG_WARNING("Truth and Predicted arrays have different unique values");

    // Print columns values first (space of 1 tab between start and between them)
    printf("\t\tPredicted\n");
    printf("True\t");
    for (size_t i = 0; i < unqPredSz; i++)
        printf("%d\t\t", uniquePred[i]);
    putchar('\n');
    for (size_t i = 0; i < cmRows; i++) {
        printf("%d\t", uniqueTrue[i]);
        for (size_t j = 0; j < cmCols; j++)
            printf("%lf\t", cm[i][j]);
        putchar('\n');
    }

    free(uniqueTrue); free(uniquePred);
    for (size_t i = 0; i < cmRows; i++)
        free(cm[i]);
    free(cm);
}
