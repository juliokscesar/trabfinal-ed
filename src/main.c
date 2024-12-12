#include <stdio.h>

#include "markov.h"

int main(void) {
    /// MARKOV TRANSITION MATRIX TEST
    printf("\n--------------------------- INIT TEST RUN ---------------------------\n");
    const int data[] = {1,1,1,0,1,1,1,0,1,1,1,0};
    const size_t n = sizeof(data) / sizeof(data[0]);

    // Build MarkovState with two values {0,1}
    uint order = 3;
    int vals[2] = {0, 1};
    const size_t nVals = sizeof(vals) / sizeof(vals[0]);
    MarkovState* state = markovBuildStates(order, vals, nVals);

    // Build markov transition matrix given the states
    TransitionMatrix* tm = markovBuildTransMatrix(data, n, state);
    markovPrintTransMatrix(tm);

    printf("DATA: ");
    for (size_t i = 0; i < n; i++)
        printf("%d, ", data[i]);
    int steps[9] = {0};
    int s = sizeof(steps)/sizeof(steps[0]);
    printf("\nNext %d steps: ", s);
    for (int i = 0; i < s; i++)
        steps[i] = markovPredict(tm, i+1, data, n);

    for (int i = 0; i < s; i++)
        printf("%d ", steps[i]);
    putchar('\n');

    markovFreeTransMatrix(&tm);
    markovFreeState(&state);
    return 0;
}
