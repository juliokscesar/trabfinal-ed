#include "markovnetwork.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "logging.h"
#include "utils.h"

/* ----------------------------- MATRIX NODE ----------------------------- */
MatrixNode* mxNodeInit(const size_t id, TransitionMatrix* matrix) {
    MatrixNode* node = malloc(sizeof(MatrixNode));
    if (!node) {
        LOG_ERROR("malloc error for node in mxNodeInit");
        return NULL;
    }

    node->id = id;
    node->matrix = matrix;

    return node;
}

void mxNodeFree(MatrixNode** node) {
    if (!node || !(*node))
        return;

    if ((*node)->matrix)
        markovFreeTransMatrix(&(*node)->matrix);
    free(*node);
    *node = NULL;
}

size_t mxNodeId(const MatrixNode* node) {
    if (!node)
        return 0;
    return node->id;
}

TransitionMatrix* mxNodeMatrix(const MatrixNode* node) {
    if (!node)
        return NULL;
    return node->matrix;
}
/* ----------------------------------------------------------------------- */

/* ----------------------------- INPUT/OUTPUT NODES/EDGES ----------------------------- */
InputNode* mkNetInitInput(const size_t id, const int* data, const size_t n) {
    InputNode* node = malloc(sizeof(InputNode));
    if (!node) {
        LOG_ERROR("malloc failed for input node");
        return NULL;
    }

    node->data = NULL;
    if (data) {
        node->data = malloc(sizeof(int) * n);
        memcpy(node->data, data, sizeof(int) * n);
    }
    node->n = 0;
    node->id = id;

    return node;
}

void mkNetFreeInput(InputNode** node) {
    if (!node || !(*node))
        return;
    if ((*node)->data)
        free((*node)->data);
    free(*node);
    *node = NULL;
}

void mkNetSetInputData(InputNode* node, const int* data, size_t n) {
    if (!node || !data)
        return;

    if (node->data)
        free(node->data);
    node->data = malloc(sizeof(int) * n);
    memcpy(node->data, data, sizeof(int) * n);
    node->n = n;
}

InputEdge* mkNetInitInEdge(InputNode* orig, MatrixNode* dest, double errFac, MKErrFuncT errFunc) {
    if (!orig || !dest)
        return NULL;

    InputEdge* edge = malloc(sizeof(InputEdge));
    if (!edge) {
        LOG_ERROR("malloc failed for input edge");
        return NULL;
    }

    edge->orig = orig;
    edge->dest = dest;
    edge->errFac = errFac;
    edge->errFunc = errFunc;

    return edge;
}

void mkNetFreeInEdge(InputEdge** edge) {
    if (!edge || !(*edge))
        return;
    free(*edge);
    *edge = NULL;
}

OutputNode* mkNetInitOutput(const size_t id, const int* vals, size_t nVals) {
    OutputNode* node = malloc(sizeof(OutputNode));
    if (!node) {
        LOG_ERROR("malloc failed for output node");
        return NULL;
    }
    node->id = id;
    node->nVals = nVals;
    node->vals = malloc(sizeof(int) * nVals);
    memcpy(node->vals, vals, sizeof(int) * nVals);

    node->probabilities = calloc(nVals, sizeof(double));
    if (!node->probabilities) {
        LOG_ERROR("calloc failed for output node probabilities vector");
        free(node);
        return NULL;
    }

    return node;
}

void mkNetFreeOutput(OutputNode** node) {
    if (!node || !(*node))
        return;

    if ((*node)->probabilities)
        free((*node)->probabilities);
    if ((*node)->vals)
        free((*node)->vals);
    free(*node);
    *node = NULL;
}

OutputEdge* mkNetInitOutEdge(MatrixNode* orig, OutputNode* dest, double weight) {
    if (!orig || !dest)
        return NULL;

    OutputEdge* edge = malloc(sizeof(OutputEdge));
    if (!edge) {
        LOG_ERROR("malloc failed for output edge");
        return NULL;
    }

    edge->orig = orig;
    edge->dest = dest;
    edge->weight = weight;

    return edge;
}

void mkNetFreeOutEdge(OutputEdge** edge) {
    if (!edge || !(*edge))
        return;
    free(*edge);
    *edge = NULL;
}

lli mkNetOutIdVal(OutputNode* node, int val) {
    if (!node)
        return -1;

    for (lli i = 0; i < node->nVals; i++) {
        if (node->vals[i] == val)
            return i;
    }
    return -1;
}

/* ------------------------------------------------------------------------------------ */

/* ----------------------------- MARKOV NETWORK ----------------------------- */
MarkovNetwork* mkNetInit(MarkovState* state, const size_t nNodes, const double* errFactors, MKErrFuncT errFunc) {
    if (!state)
        return NULL;

    MarkovNetwork* net = malloc(sizeof(MarkovNetwork));
    if (!net) {
        LOG_ERROR("malloc failed for markov network");
        return NULL;
    }

    net->start = mkNetInitInput(0, NULL, 0);
    net->end = mkNetInitOutput(0, state->vals, state->nVals);
    net->markovOrder = state->order;

    net->nMatNodes = nNodes;
    net->input = calloc(nNodes, sizeof(InputEdge*));
    net->output = calloc(nNodes, sizeof(OutputEdge*));
    for (size_t i = 0; i < nNodes; i++) {
        MatrixNode* mx = mxNodeInit(i, markovInitTransMatrix(NULL, state));
        double errFac = (errFactors) ? errFactors[i] : 0.0;
        net->input[i] = mkNetInitInEdge(net->start, mx, errFac, errFunc);
        net->output[i] = mkNetInitOutEdge(mx, net->end, 1.0);
    }

    return net;
}

void mkNetFree(MarkovNetwork** net) {
    if (!net || !(*net))
        return;

    if ((*net)->start)
        mkNetFreeInput(&(*net)->start);
    if ((*net)->end)
        mkNetFreeOutput(&(*net)->end);

    if ((*net)->input && (*net)->output) {
        for (size_t i = 0; i < (*net)->nMatNodes; i++) {
            // free matrix nodes
            if ((*net)->input[i] && (*net)->input[i]->dest)
                mxNodeFree(&(*net)->input[i]->dest);
            if ((*net)->input[i])
                mkNetFreeInEdge(&(*net)->input[i]);
            if ((*net)->output[i])
                mkNetFreeOutEdge(&(*net)->output[i]);
        }
    }
    free((*net)->input);
    free((*net)->output);
    free(*net);
    *net = NULL;
}

void mkNetMatrixNodes(MarkovNetwork* net, MatrixNode** out) {
    if (!net || !out)
        return;

    for (size_t i = 0; i < net->nMatNodes; i++)
        out[i] = net->input[i]->dest;
}

void mkNetTrain(MarkovNetwork* net, int* train, const size_t trainSize, const int* valid, const size_t validSize, const double lr) {
    // The training process is:
    // 1. First, train each matrix with their respective input errors, using the 'train' set
    // 2. Forward the 'valid' set to get the output of each node separately
    // 3. Backward the results to calculate the error
    // 4. Update the weights accordingly
    if (!net || !train || !valid)
        return;

    // Train initial matrices
    mkNetSetInputData(net->start, train, trainSize);
    mkNetInitMatrices(net);

    // Go through each value of the 'valid' set
    // and compare it with the predicted output of the node
    // then increase its weight if ok, else decrease
    int* prediction = malloc(sizeof(int) * validSize);
    for (size_t i = 0; i < net->nMatNodes; i++) {
        MatrixNode* currNode = net->output[i]->orig;

        markovPredict(currNode->matrix, (uint)validSize, net->start->data, net->start->n, prediction, NULL);
        for (size_t v = 0; v < validSize; v++)
            mkNetUpdateWeights(net, lr, i, valid[v] == prediction[v]);
    }
    free(prediction);

    // normalize the weights at the end
    //mkNetNormStd(net);
    mkNetNormSoftmax(net, 1.0);

    // then update data to include only the last state from valid, otherwise it will have old data (from train) and not from valid
    mkNetSetLastState(net, &valid[validSize - net->markovOrder]);
}

void mkNetInitMatrices(MarkovNetwork* net) {
    if (!net)
        return;

    // copy train data, because we may introduce some error in it
    int* trainCopy = malloc(sizeof(int) * net->start->n);
    memcpy(trainCopy, net->start->data, sizeof(int) * net->start->n);

    // Train with train set, with some random error applied
    // *****CHANGE: introduce error in matrix not data*****
    for (size_t i = 0; i < net->nMatNodes; i++) {
        const InputEdge* inEdge = net->input[i];
        // apply error if any
        if (inEdge->errFac > 0.0) {
            inEdge->errFunc(inEdge->dest->id, net->start->data,  trainCopy, net->start->n, inEdge->errFac);
            markovFillProbabilities(inEdge->dest->matrix, trainCopy, net->start->n);
        }
        else
            markovFillProbabilities(inEdge->dest->matrix, net->start->data, net->start->n);
    }

    free(trainCopy);
}

void mkNetUpdateWeights(MarkovNetwork* net, const double lr, const size_t id, bool correct) {
    if (!net)
        return;

    OutputEdge* edge = net->output[id];
    if (correct)
        edge->weight += lr;
    else
        edge->weight -= lr;

    // constrain to values between 0.0 and 1.0 (they are normalized in the training function)
    if (edge->weight < 0.0)
        edge->weight = 0.0;
}

void mkNetNormStd(MarkovNetwork* net) {
    // Standard Normalization: Wnorm_i = W_i / sum(W)
    double weightSum = 0.0;
    for (size_t i = 0; i < net->nMatNodes; i++)
        weightSum += net->output[i]->weight;

    for (size_t i = 0; i < net->nMatNodes; i++)
        net->output[i]->weight /= weightSum;
}

void mkNetNormSoftmax(MarkovNetwork* net, double temperature) {
    // Softmax normalization: Wnorm = exp(W_i / T) / sum(exp(W)))
    double* expBuffer = malloc(sizeof(double) * net->nMatNodes);
    double expSum = 0.0;
    for (size_t i = 0; i < net->nMatNodes; i++) {
        expBuffer[i] = exp(net->output[i]->weight / temperature);
        expSum += expBuffer[i];
    }

    for (size_t i = 0; i < net->nMatNodes; i++)
        net->output[i]->weight = expBuffer[i] / expSum;

    free(expBuffer);
}

void mkNetSetLastState(MarkovNetwork* net, const int* lastState) {
    if (!net || !lastState)
        return;
    mkNetSetInputData(net->start, lastState, net->markovOrder);
}

void mkNetPredict(MarkovNetwork* net, const size_t steps, int* predOut, double* confOut) {
    // The prediction process is:
    // 1. Get the output of each node separately
    // 2. The probability of the value 0 to be the next will be the sum of the weights of every node that answered 0 (or weight*probability)
    // 3. Then set the final answer to be that with the highest sum
    if (!net || !predOut)
        return;

    // first reset output probabilities
    memset(net->end->probabilities, 0, sizeof(double) * net->end->nVals);

    // keep track of the last state only
    int* lastState = malloc(sizeof(int) * net->markovOrder);
    memcpy(lastState, net->start->data + net->start->n - net->markovOrder, sizeof(int) * net->markovOrder);

    int prediction = INT_MIN;
    double maxProb = 0.0;
    for (size_t i = 0; i < steps; i++) {
        // Get every node's answer
        for (size_t o = 0; o < net->nMatNodes; o++) {
            double prob = 0.0;
            int pred = markovPredictNext(net->output[o]->orig->matrix, lastState, net->markovOrder, &prob);
            lli valID = mkNetOutIdVal(net->output[o]->dest, pred);
            if (valID == -1) {
                LOG_ERROR("Unable to identify value in mkNetPredict.");
                fprintf(stderr, "pred=%d, valid=%ld, net->output[o]->dest->nVals=%zu, lastState: ", pred, valID, net->output[o]->dest->nVals);
                printArr_i(lastState, net->markovOrder);
            }
            else
                net->end->probabilities[valID] += net->output[o]->weight*prob;
        }

        // Chose prediction by argmax
        prediction = net->end->vals[0];
        for (size_t v = 0; v < net->end->nVals; v++) {
            if (net->end->probabilities[v] > maxProb) {
                maxProb = net->end->probabilities[v];
                prediction = net->end->vals[v];
            }
        }

        // Update last state in the end
        for (size_t s = 0; s < (net->markovOrder-1); s++)
            lastState[s] = lastState[s+1];
        lastState[net->markovOrder-1] = prediction;

        predOut[i] = prediction;
        if (confOut)
            confOut[i] = maxProb;
        // reset values
        prediction = INT_MIN;
        maxProb = 0.0;
        memset(net->end->probabilities, 0, sizeof(double) * net->end->nVals);
    }

    free(lastState);
}

size_t mkNetOptimalNode(const MarkovNetwork* net, const double alpha, double* score) {
    if (!net)
        return INT_MAX;

    // First get maximum and minimum weight and error factor
    double wmax = (double)INT_MIN;
    double wmin = (double)INT_MAX;
    double errmax = wmax;
    double errmin = wmin;

    for (size_t i = 0; i < net->nMatNodes; i++) {
        if (net->input[i]->errFac > errmax)
            errmax = net->input[i]->errFac;
        if (net->input[i]->errFac < errmin)
            errmin = net->input[i]->errFac;

        if (net->output[i]->weight > wmax)
            wmax = net->output[i]->weight;
        if (net->output[i]->weight < wmin)
            wmin = net->output[i]->weight;
    }

    *score = (double)INT_MIN;
    size_t optimalID = net->nMatNodes;
    for (size_t i = 0; i < net->nMatNodes; i++) {
        double wnorm = (net->output[i]->weight - wmin) / (wmax - wmin);
        double errnorm = (net->input[i]->errFac - errmin) / (errmax - errmin);

        double nodeScore = alpha * wnorm - (1.0-alpha) * errnorm;
        if (nodeScore > *score) {
            *score = nodeScore;
            optimalID = i;
        }
    }

    return optimalID;
}

void mkNetExport(const MarkovNetwork* net, const char* file) {
    if (!net || !file)
        return;

    FILE* out = fopen(file, "w");
    if (!out) {
        LOG_ERROR("Unable to open file to export markov network");
        return;
    }

    fprintf(out, "digraph MarkovNetwork {\n");
    fprintf(out, "    rankdir=LR;\n");  // Left-to-right layout

    // Print Input Node
    fprintf(out, "    \"Input\" [shape=ellipse, label=\"Input\\n(state vector)\"];\n");

    // Print Matrix Nodes
    for (size_t i = 0; i < net->nMatNodes; i++) {
        fprintf(out, "    \"MatrixNode_%zu\" [shape=box, label=\"MatrixNode %zu\\n(Transition Matrix)\"];\n", i, i);
    }

    // Print Output Node
    fprintf(out, "    \"Output\" [shape=ellipse, label=\"Output\\n(Final Probabilities)\"];\n");

    // Print InputEdges
    for (size_t i = 0; i < net->nMatNodes; i++) {
        fprintf(out, "    \"Input\" -> \"MatrixNode_%zu\" [label=\"errorFactor=%.2f\"];\n",
                i, net->input[i]->errFac);
    }

    // Print OutputEdges
    for (size_t i = 0; i < net->nMatNodes; i++) {
        fprintf(out, "    \"MatrixNode_%zu\" -> \"Output\" [label=\"weight=%.2f\"];\n",
                i, net->output[i]->weight);
    }

    fprintf(out, "}\n");
    fclose(out);

    LOG_INFO("Markov Network exported to file:");
    fprintf(stderr, "%s\n", file);
}
/* -------------------------------------------------------------------------- */

/* -------------------------- USEFUL ERROR FUNCTIONS -------------------------- */
void randomBinarySwap(size_t nodeId, const int* data, int* out, size_t n, double errFactor) {
    for (size_t i = 0; i < n; i++) {
        if (rand01_d() <= errFactor)
            out[i] = 1 - data[i];
        else
            out[i] = data[i];
    }
}

void binarySegmentNoise(size_t nodeId, const int* data, int* out, size_t n, double errFactor) {
    const size_t SEG_LEN = 3;
    for (size_t i = 0; i < n; i++) {
        out[i] = data[i];
        if (i + SEG_LEN > n)
            continue;

        if (rand01_d() <= errFactor) {
            for (size_t j = i; j < i + SEG_LEN; j++)
                out[j] = 1 - data[j];
            i += SEG_LEN - 1;
        }
    }
}

void randomSwap(size_t nodeId, const int* data, int* out, size_t n, double errFactor) {
    int* unique = NULL;
    size_t nUnique = 0;
    findDistinct_i(data, n, &unique, &nUnique);

    for (size_t i = 0; i < n; i++) {
        if (rand01_d() <= errFactor)
            out[i] = unique[ rand() % n ];
        else
            out[i] = data[i];
    }

    free(unique);
}
/* ---------------------------------------------------------------------------- */
