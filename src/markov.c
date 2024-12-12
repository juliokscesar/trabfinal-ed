#include "markov.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "utils.h"

MarkovState* markovBuildStates(const uint order, const int* vals, size_t nVals) {
    MarkovState* state = malloc(sizeof(MarkovState));
    state->order = order;

    // Initialize the values alphabet (will be mostly 0,1)
    state->nVals = nVals;
    state->vals = malloc(sizeof(int) * nVals);
    memcpy(state->vals, vals, nVals * sizeof(int));

    // Allocate memory for states
    state->nStates = (size_t)pow((double)nVals, (double)order);
    state->states = calloc(state->nStates, sizeof(int*));
    for (size_t i = 0; i < state->nStates; i++)
        state->states[i] = calloc(order, sizeof(int));

    // Initialize states
    // They are the N^order combinations of the values
    size_t comb = 0;
    ibuildCombinations(vals, nVals, state->order, state->states, &comb);

    return state;
}

void markovFreeState(MarkovState** state) {
    if (!state || !(*state))
        return;

    // Free vals
    free((*state)->vals);

    // Free states
    for (size_t i = 0; i < (*state)->nStates; i++)
        free((*state)->states[i]);
    free((*state)->states);

    // Free the state pointer and set it to NULL
    free(*state);
    *state = NULL;
}

lli markovIdState(const MarkovState* state, const int* stateVec) {
    if (!state || !stateVec)
        return -1;

    for (size_t stateID=0; stateID < state->nStates; stateID++) {
        bool match = true;
        for (size_t i=0; i < state->order; i++) {
            if (state->states[stateID][i] != stateVec[i]) {
                match = false;
                break;
            }
        }
        if (match)
            return (lli)stateID;
    }

    return -1;
}

lli markovIdValState(const MarkovState* state, const int val) {
    if (!state)
        return -1;

    for (size_t valID = 0; valID < state->nVals; valID++) {
        if (state->vals[valID] == val)
            return (lli)valID;
    }

    return -1;
}


TransitionMatrix* markovInitTransMatrix(const double** probs, MarkovState* state) {
    if (!probs || !state)
        return NULL;

    TransitionMatrix* m = malloc(sizeof(TransitionMatrix));
    m->state = state;
    m->probs = calloc(state->nStates, sizeof(double*));
    for (size_t i = 0; i < state->nStates; i++) {
        m->probs[i] = calloc(state->nVals, sizeof(double));
        memcpy(m->probs[i], probs[i], state->nVals * sizeof(double));
    }

    return m;
}

TransitionMatrix* markovBuildTransMatrix(const int* data, const size_t n, MarkovState* state) {
    if (!data || !state)
        return NULL;

    TransitionMatrix* m = malloc(sizeof(TransitionMatrix));
    m->state = state;

    // The probability matrix will have dimension N_States X N_Values
    m->probs = calloc(state->nStates, sizeof(double*));
    for (size_t i = 0; i < state->nStates; i++)
        m->probs[i] = calloc(state->nVals, sizeof(double));

    markovFillProbabilities(m, data, n);

    return m;
}

void markovFreeTransMatrix(TransitionMatrix** m) {
    if (!m || !(*m))
        return;

    // Don't free state because it can be shared

    // Free probabilities
    for (size_t i = 0; i < (*m)->state->nStates; i++)
        free((*m)->probs[i]);
    free((*m)->probs);

    // Finally free TM pointer and set it to NULL
    free(*m);
    *m = NULL;
}

void markovFillProbabilities(TransitionMatrix* m, const int* data, const size_t n) {
    if (!m || !data)
        return;
    if (m->state->order > (n-1))
        return;

    // Look for occurences of every state in the data
    for (size_t stateID = 0; stateID < m->state->nStates; stateID++) {
        for (size_t dataI = 0; dataI < n; dataI++) {
            // Get the starting index of the next occurrence of the state
            size_t stateInData = ifindSubsetIn(data, n, dataI, m->state->states[stateID], m->state->order);
            if (stateInData == -1)
                break;

            // If we got a match, update the next value count in the probablity matrix
            int nextVal = data[stateInData+m->state->order];
            lli valID = markovIdValState(m->state, nextVal);
            if (valID == -1)
                continue;
            m->probs[stateID][valID]++;
            dataI = stateInData;
        }

        // After counting, normalize the probabilities
        double stateCount = 0.0;
        for (size_t i = 0; i < m->state->nVals; i++) {
            stateCount += m->probs[stateID][i];
        }
        if (stateCount > 0.0) {
            for (size_t i = 0; i < m->state->nVals; i++)
                m->probs[stateID][i] /= stateCount;
        }
    }
}

void markovPrintTransMatrix(const TransitionMatrix* m) {
    if (!m)
        return;

    // First print 'ID0 ID1 ...' for values
    putchar('\t');
    for (size_t v = 0; v < m->state->nVals; v++)
        printf("%d\t\t\t", m->state->vals[v]);
    putchar('\n');

    // Now print state, p1, p2...
    for (size_t s = 0; s < m->state->nStates; s++) {
        for (size_t o = 0; o < m->state->order; o++)
            printf("%d", m->state->states[s][o]);
        putchar('\t');
        for (size_t v = 0; v < m->state->nVals; v++)
            printf("%lf\t", m->probs[s][v]);
        putchar('\n');
    }
}

int markovPredict(const TransitionMatrix* m, const uint steps, const int* data, const size_t n) {
    if (!m || !data || !m->state)
        return INT_MAX;
    if (m->state->order > (n-1))
        return INT_MAX;

    // The last state will be the slice [n-order:]
    int* lastState = malloc(sizeof(int)*m->state->order);
    for (size_t i = n-m->state->order; i < n; i++)
        lastState[i-n+m->state->order] = data[i];

    lli stateID = markovIdState(m->state, lastState);
    if (stateID == -1)
        return INT_MAX;

    // Update 'lastState' with every step
    int prediction = m->state->vals[0];
    for (uint i = 0; i < steps; i++) {
        // predict the next value with the given state
        // to do that, generate random number between 0 and 1
        // then the next value will have the probability between p(s) <= r < p(s+1)
        double r = drand01();
        stateID = markovIdState(m->state, lastState);

        double low = 0.0;
        for (size_t v = 0; v < m->state->nVals; v++) {
            double high = m->probs[stateID][v];
            if (r >= low && r < high) {
                prediction = m->state->vals[v];
                break;
            }
            low = high;
        }

        // Then update the last state to include this new value
        for (size_t j = 0; j < m->state->order-1; j++)
            lastState[j] = lastState[j+1];
        lastState[m->state->order - 1] = prediction;
    }

    free(lastState);

    return prediction;
}
