#include "markov.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "utils.h"
#include "logging.h"

MarkovState* markovBuildStates(const uint order, const int* vals, size_t nVals) {
    MarkovState* state = malloc(sizeof(MarkovState));
    if (!state) {
        LOG_ERROR("malloc failed for MarkovState*");
        return NULL;
    }
    state->order = order;

    // Initialize the values alphabet (will be mostly 0,1)
    state->nVals = nVals;
    state->vals = malloc(sizeof(int) * nVals);
    if (!state->vals) {
        LOG_ERROR("malloc failed for MarkovState values (state->vals)");
        free(state);
        return NULL;
    }
    memcpy(state->vals, vals, nVals * sizeof(int));

    // Allocate memory for states
    state->nStates = (size_t)pow((double)nVals, (double)order);
    state->states = malloc(state->nStates * sizeof(int*));
    if (!state->states) {
        LOG_ERROR("malloc failed for MarkovState states combinations (state->states)");
        free(state->vals);
        free(state);
        return NULL;
    }
    for (size_t i = 0; i < state->nStates; i++) {
        state->states[i] = calloc(order, sizeof(int));
        if (!state->states[i]) {
            LOG_ERROR("calloc failed for one of the state in MarkovState state->states");
            for (size_t j = 0; j < i; j++)
                free(state->states[j]);
            free(state->states);
            free(state->vals);
            free(state);
            return NULL;
        }
    }

    // Initialize states
    // They are the N^order combinations of the values
    size_t comb = 0;
    buildCombinations_i(vals, nVals, state->order, state->states, &comb);

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
    if (!state || !stateVec || !state->states)
        return -1;

    for (size_t stateID=0; stateID < state->nStates; stateID++) {
        if (memcmp(stateVec, state->states[stateID], sizeof(int) * state->order) == 0)
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
    if (!state)
        return NULL;

    TransitionMatrix* m = malloc(sizeof(TransitionMatrix));
    if (!m) {
        LOG_ERROR("malloc failed for TransitionMatrix* m");
        return NULL;
    }

    m->state = state;
    if (probs) {
        m->probs = malloc(state->nStates * sizeof(double*));
        if (!m->probs) {
            LOG_ERROR("malloc failed for probabilities matrix m->probs");
            free(m);
            return NULL;
        }
        for (size_t i = 0; i < state->nStates; i++) {
            m->probs[i] = malloc(state->nVals * sizeof(double));
            if (!m->probs[i]) {
                LOG_ERROR("malloc failed for one of probabilities rows");
                for (size_t j = 0; j < i; j++)
                    free(m->probs[j]);
                free(m->probs);
                free(m);
                return NULL;
            }
            memcpy(m->probs[i], probs[i], state->nVals * sizeof(double));
        }
    } else {
        m->probs = NULL;
    }

    return m;
}

TransitionMatrix* markovBuildTransMatrix(const int* data, const size_t n, MarkovState* state) {
    if (!data || !state)
        return NULL;

    TransitionMatrix* m = malloc(sizeof(TransitionMatrix));
    if (!m) {
        LOG_ERROR("malloc failed for TransitionMatrix* m");
        return NULL;
    }
    m->state = state;

    // The probability matrix will have dimension N_States X N_Values
    m->probs = malloc(state->nStates * sizeof(double*));
    if (!m->probs) {
        LOG_ERROR("malloc failed for probabilities matrix m->probs");
        free(m);
        return NULL;
    }
    for (size_t i = 0; i < state->nStates; i++) {
        m->probs[i] = malloc(state->nVals * sizeof(double));
        if (!m->probs[i]) {
            LOG_ERROR("malloc failed for one of probabilities rows");
            for (size_t j = 0; j < i; j++)
                free(m->probs[j]);
            free(m->probs);
            free(m);
            return NULL;
        }
    }

    markovFillProbabilities(m, data, n);

    return m;
}

void markovFreeTransMatrix(TransitionMatrix** m) {
    if (!m || !(*m) || !(*m)->probs)
        return;

    // Don't free state because it may be shared

    // Free probabilities
    for (size_t i = 0; i < (*m)->state->nStates; i++)
        free((*m)->probs[i]);
    free((*m)->probs);

    // Finally free TM pointer and set it to NULL
    free(*m);
    *m = NULL;
}

void markovFillProbabilities(TransitionMatrix* m, const int* data, const size_t n) {
    if (!m || !data || !m->state)
        return;
    if (m->state->order > (n-1))
        return;

    if (!m->probs) {
        m->probs = calloc(m->state->nStates, sizeof(double*));
        for (size_t i = 0; i < m->state->nStates; i++)
            m->probs[i] = calloc(m->state->nVals, sizeof(double));
    }

    // For every state size N, create a vector of N+1 size and count the occurrences of that vector
    // since it will be the number of times that state transitioned to the (N+1) value
    const size_t stateVecSize = m->state->order + 1;
    int* stateVec = malloc(sizeof(int) * stateVecSize);
    for (size_t stateID = 0; stateID < m->state->nStates; stateID++) {
        // copy first (order) values to statevec
        memcpy(stateVec, m->state->states[stateID], sizeof(int)*m->state->order);

        // then, the last value will be a possible value for the transition (from m->state->values)
        uint total = 0;
        for (size_t valID = 0; valID < m->state->nVals; valID++) {
            stateVec[stateVecSize-1] = m->state->vals[valID];
            uint count = countSubsetIn_i(data, n, stateVec, stateVecSize);
            m->probs[stateID][valID] = count;
            total += count;
        }

        // finally, normalize the probabilities
        if (total > 0) {
            for (size_t valID = 0; valID < m->state->nVals; valID++)
                m->probs[stateID][valID] /= total;
        }
    }

    free(stateVec);
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

void markovPredict(const TransitionMatrix* m, const uint steps, const int* data, const size_t n, int* predOut, double* confOut) {
    if (!m || !data || !m->state || !predOut)
        return;
    if (m->state->order > n)
        return;

    // The last state will be the slice [n-order:]
    int* lastState = malloc(sizeof(int)*m->state->order);
    if (!lastState) {
        LOG_ERROR("malloc failed for vector of last state");
        return;
    }
    for (size_t i = n-m->state->order; i < n; i++)
        lastState[i-n+m->state->order] = data[i];
    memcpy(lastState, data + n - m->state->order, sizeof(int) * m->state->order);

    lli stateID = markovIdState(m->state, lastState);
    if (stateID == -1) {
        LOG_ERROR("Unable to identiy id by value: ");
        printf("%ld\n", stateID);
        free(lastState);
        return;
    }

    // Update 'lastState' with every step
    int prediction = m->state->vals[0];
    for (uint i = 0; i < steps; i++) {
        // predict the next value with the given state
        // to do that, generate random number between 0 and 1
        // then the next value will have the probability between p(s) <= r < p(s+1)
        double r = rand01_d();
        stateID = markovIdState(m->state, lastState);

        double cumProb = 0.0;
        for (size_t v = 0; v < m->state->nVals; v++) {
            cumProb += m->probs[stateID][v];
            if (r <= cumProb) {
                prediction = m->state->vals[v];
                if (confOut)
                    confOut[i] = cumProb;
                break;
            }
        }

        // Then update the last state to include this new value
        for (size_t j = 0; j < m->state->order-1; j++)
            lastState[j] = lastState[j+1];
        lastState[m->state->order - 1] = prediction;
        predOut[i] = prediction;
    }

    free(lastState);
}

int markovPredictNext(const TransitionMatrix* m, const int* data, const size_t n) {
    if (!m || !data)
        return INT_MAX;

    int prediction = INT_MAX;
    lli stateID = markovIdState(m->state, data + n - m->state->order);
    if (stateID == -1) {
        LOG_ERROR("Unable to identify state");
        printf("ID: %ld, State: ", stateID);
        printArr_i(data+n-m->state->order, m->state->order);
    }

    double r = rand01_d();
    double cumProb = 0.0;
    for (size_t v = 0; v < m->state->nVals; v++) {
        cumProb += m->probs[stateID][v];
        if (r <= cumProb) {
            prediction = m->state->vals[v];
            break;
        }
    }

    return prediction;
}
