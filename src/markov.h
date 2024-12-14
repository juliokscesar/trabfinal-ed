#ifndef _MARKOV_H

#include "typedefs.h"

// Markov State
// Keeps the states vectors like [0],[1] for order 1, or [0,0],[1,0]... for order 2 and so on
typedef struct {
    uint order;
    int** states;
    size_t nStates;
    int* vals;
    size_t nVals;
} MarkovState;

MarkovState* markovBuildStates(const uint order, const int* vals, size_t nVals);
void markovFreeState(MarkovState** state);
lli markovIdState(const MarkovState* state, const int* stateVec);
lli markovIdValState(const MarkovState* state, const int val);

// Markov Transition Matrix
// each row of 'probs' represents a current state.
// each column represents the next value.
// So probs[s][ID] is the probability of the next value being ID given the current state s
typedef struct {
    MarkovState* state;
    double** probs;
} TransitionMatrix;

// Initialize transition matrix with custom probabilities and states
//
TransitionMatrix* markovInitTransMatrix(const double** probs, MarkovState* state);

// Build transition matrix based on time series from the data
// If 'stateVals' is NULL, the values (ids) are set from 0 to nVals
TransitionMatrix* markovBuildTransMatrix(const int* data, const size_t n, MarkovState* state);

// Free the allocated memory for *m and set *m to NULLs
void markovFreeTransMatrix(TransitionMatrix** m);

void markovFillProbabilities(TransitionMatrix* m, const int* data, const size_t n);

// Print transition matrix in a matrix format, like:
/*     ID0 ID1
 * ID0 P00 P01
 * ID1 P10 P11
 */
void markovPrintTransMatrix(const TransitionMatrix* m);

// Predicts the next 'steps' time steps based on the probabilities in the TransitionMatrix
// given the last state
void markovPredict(const TransitionMatrix* m, const uint steps, const int* data, const size_t n, int* predOut, double* confOut);

// Predict next step
int markovPredictNext(const TransitionMatrix* m, const int* data, const size_t n);

#endif // _MARKOV_H