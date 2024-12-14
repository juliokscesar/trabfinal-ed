#include <stdio.h>
#include <stdlib.h>

#include "markov.h"
#include "logging.h"
#include "markovgraph.h"
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
    uint order = 6;
    int vals[] = {0,1};
    const size_t nVals = sizeof(vals) / sizeof(vals[0]);
    MarkovState* state = markovBuildStates(order, vals, nVals);
    printf("Using order %u, number of states: %ld\n", order, state->nStates);

    // Build markov transition matrix given the states and the training set
    TransitionMatrix* tm = markovBuildTransMatrix(train, trainSize, state);
    //markovPrintTransMatrix(tm);

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
    markovFreeState(&state);
    free(predicted);
    free(train);
    free(test);
    free(data);
    return 0;
}
