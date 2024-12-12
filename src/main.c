#include <stdio.h>
#include <stdlib.h>

#include "markov.h"
#include "logging.h"

int main(void) {
    /// MARKOV TRANSITION MATRIX TEST
    printf("\n--------------------------- INIT TEST RUN ---------------------------\n");
    srand(42);

    const int data[] = {1,2,21,16,1,2,21,16,1,2,21,16};
    const size_t n = sizeof(data) / sizeof(data[0]);

    // Build MarkovState with two values {0,1}
    uint order = 3;
    int vals[] = {0,1};
    const size_t nVals = sizeof(vals) / sizeof(vals[0]);
    MarkovState* state = markovBuildStates(order, vals, nVals);

    // Build markov transition matrix given the states
    TransitionMatrix* tm = markovBuildTransMatrix(data, n, state);
    markovPrintTransMatrix(tm);

    printf("DATA: ");
    for (size_t i = 0; i < n; i++)
        printf("%d, ", data[i]);
    int steps[10] = {0};
    double conf[10] = {0};
    const int s = sizeof(steps)/sizeof(steps[0]);
    printf("\nNext %d steps: ", s);
    for (int i = 0; i < s; i++) {
        steps[i] = markovPredict(tm, i+1, data, n, &conf[i]);
    }

    // Print previsao e confidence
    for (int i = 0; i < s; i++) {
        printf("%d(%lf) ", steps[i], conf[i]);
    }
    double prop = conf[0];
    printf("\nAcc confidence:  ");
    for (int i = 0; i < s; i++) {
        if (i!=0)
            prop *= conf[i];
        printf("%lf;   ", prop);
    }
    putchar('\n');
    // ------------------------------------

    markovFreeTransMatrix(&tm);
    markovFreeState(&state);
    return 0;
}
