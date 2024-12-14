#include <stdio.h>
#include <stdlib.h>

#include "markov.h"
#include "logging.h"
#include "markovgraph.h"
#include "markovnetwork.h"
#include "utils.h"

int main(int argc, char** argv) {
    /// MARKOV TRANSITION MATRIX TEST
    printf("\n--------------------------- INIT TEST RUN ---------------------------\n");
    srand(42);

    size_t n = 0;
    int* data = NULL;
    if (argc > 1)
        data = loadData_i(argv[1], &n);
    else
        data = loadData_i("../test.dat", &n);
    if (!data) {
        LOG_ERROR("Unable to load test.dat");
        return -1;
    }

    int *train, *test;
    size_t trainSize, testSize;
    splitTrainTest_i(data, n, &train, &test, &trainSize, &testSize, 0.3);
    if (!train || !test) {
        LOG_ERROR("Couldn't split train and test");
        free(data);
        return -1;
    }

    printf("Data N = %ld\n", n);
    printf("Training set (%ld): ", trainSize);
    printArr_i(train, trainSize);
    printf("Test set (%ld): ", testSize);
    printArr_i(test, testSize);

    // Build MarkovState with two values {0,1}
    uint order = 3;
    int vals[] = {0,1};
    const size_t nVals = sizeof(vals) / sizeof(vals[0]);
    MarkovState* state = markovBuildStates(order, vals, nVals);
    printf("Using order %u, number of states: %ld\n", order, state->nStates);

    // Build markov transition matrix given the states and the training set
    TransitionMatrix* tm = markovBuildTransMatrix(train, trainSize, state);
    markovPrintTransMatrix(tm);

    printf("\nTest set validation:\n");
    printf("Test set (truth) (%ld): ", testSize);
    printArr_i(test, testSize);

    // predict next (testSize)
    int* predicted = malloc(sizeof(int) * testSize);
    markovPredict(tm, testSize, train, trainSize, predicted, NULL);

    printf("Predicted: ");
    printArr_i(predicted, testSize);

    double acc = calcAccuracy(test, predicted, testSize);
    printf("Accuracy: %lf\n", acc);

    // Print previsao e confidence
    // for (int i = 0; i < s; i++) {
    //     printf("%d(%lf) ", steps[i], conf[i]);
    // }
    // double prop = conf[0];
    // printf("\nAcc confidence:  ");
    // for (int i = 0; i < s; i++) {
    //     if (i!=0)
    //         prop *= conf[i];
    //     printf("%lf;   ", prop);
    // }
    // putchar('\n');
    // ------------------------------------

    MarkovGraph* graph = mkGraphInit(state);
    LOG_INFO("initiated graph");
    mkGraphBuildTransitions(graph, tm);
    LOG_INFO("built transitions in graph");
    mkGraphExport(graph, "graph.dot");

    mkGraphFree(&graph);
    markovFreeTransMatrix(&tm);
    free(predicted);
    free(train);
    free(test);

    // NETWORK TEST
    printf("\n____________________________________________________\n");
    printf("NETWORK TESTING\n");
    printf("____________________________________________________\n");

    int* val;
    size_t trainsz, valsz, testsz;
    splitTrainValTest_i(data, n, &train, &val, &test, &trainsz, &valsz, &testsz, 0.4, 0.1);
    if (!train || !val || !test) {
        LOG_ERROR("Couldn't split train, val, test");
        return -1;
    }
    printf("Train set (%zu): ", trainsz);
    printArr_i(train,trainsz);
    printf("Valid set (%zu): ", valsz);
    printArr_i(val, valsz);
    printf("Test set (%zu): ", testsz);
    printArr_i(test, testsz);

    const size_t nNetNodes = 30;
    const double lr = 0.001;
    double* errorFactor = malloc(sizeof(double) * nNetNodes);
    for (size_t i = 0; i < nNetNodes; i++)
        errorFactor[i] = 0.03 * (double)i;

    MarkovNetwork* net = mkNetInit(state, nNetNodes, errorFactor, randomBinarySwap);
    mkNetTrain(net, train, trainsz, val, valsz, lr);
    LOG_INFO("Finished training network");

    predicted = malloc(sizeof(int) * testsz);
    mkNetPredict(net, testsz, predicted, NULL);

    printf("\nTest set validation:\n");
    printf("Test set (truth) (%ld): ", testsz);
    printArr_i(test, testsz);
    printf("Predicted: ");
    printArr_i(predicted, testsz);

    acc = calcAccuracy(test, predicted, testsz);
    printf("Accuracy: %lf\n", acc);

    mkNetExport(net, "network.dot");

    mkNetFree(&net);
    markovFreeState(&state);
    free(predicted);
    free(train);
    free(test);
    free(val);
    free(errorFactor);
    free(data);
    return 0;
}
