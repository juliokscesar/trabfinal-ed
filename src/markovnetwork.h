#ifndef MARKOVNETWORK_H
#define MARKOVNETWORK_H

#include "markov.h"

/* ----------------------------- MATRIX NODE ----------------------------- */
// MatrixNode represents each node in the graph containing one TransitionMatrix
typedef struct {
   size_t id;
   TransitionMatrix* matrix;
} MatrixNode;

MatrixNode* mxNodeInit(const size_t id, TransitionMatrix* matrix);
void mxNodeFree(MatrixNode** node);
size_t mxNodeId(const MatrixNode* node);
TransitionMatrix* mxNodeMatrix(const MatrixNode* node);
/* ----------------------------------------------------------------------- */

typedef struct {
   size_t id;
   int* data;
   size_t n;
} InputNode;

typedef void(*MKErrFuncT)(size_t,const int*,int*,size_t,double);
typedef struct {
   InputNode* orig;
   MatrixNode* dest;
   double errFac;
   // errorFunc must be a function to take as input (dest->id, data, dest, size, errorFactor)
   MKErrFuncT errFunc;
} InputEdge;

InputNode* mkNetInitInput(const size_t id, const int* data, const size_t n);
void mkNetFreeInput(InputNode** node);
void mkNetSetInputData(InputNode* node, const int* data, size_t n);
InputEdge* mkNetInitInEdge(InputNode* orig, MatrixNode* dest, double errFac, MKErrFuncT errFunc);
void mkNetFreeInEdge(InputEdge** edge);

typedef struct {
   size_t id;
   size_t nVals;
   int* vals;
   double* probabilities;
} OutputNode;

typedef struct {
   MatrixNode* orig;
   OutputNode* dest;
   double weight;
} OutputEdge;

OutputNode* mkNetInitOutput(const size_t id, const int* vals, size_t nVals);
void mkNetFreeOutput(OutputNode** node);
OutputEdge* mkNetInitOutEdge(MatrixNode* orig, OutputNode* dest, double weight);
void mkNetFreeOutEdge(OutputEdge** edge);
lli mkNetOutIdVal(OutputNode* node, int val);

/* ----------------------------- MARKOV NETWORK ----------------------------- */
/* Markov Network will be a Matrix Graph applied to a 'neural network'
   Each node (neuron) contains a Transition Matrix. The first node is
   trained with the data as it is. The next nodes are trained with
   random errors, introduced to generalize views from the data.

   The graph has a first layer of M non-connected nodes. They all receive
   the data from a root node, which will contain a associated 'error function'
   to introduce to the data. The output of each node will be compared to a
   'validation set', which true values will determine if we must increase/decrease
   the weight of their output edges.

   The output edges will be connected to an 'end' node. In that node, the predicted
   value will be obtained by calculating a 'weighted vote'.
*/
typedef struct {
   InputNode* start;
   InputEdge** input;
   OutputEdge** output;
   OutputNode* end;

   size_t nMatNodes;
   uint markovOrder;
} MarkovNetwork;

MarkovNetwork* mkNetInit(MarkovState* state, const size_t nNodes, const double* errFactors, MKErrFuncT errFunc);
void mkNetFree(MarkovNetwork** net);
void mkNetMatrixNodes(MarkovNetwork* net, MatrixNode** out);

void mkNetTrain(MarkovNetwork* net, int* train, const size_t trainSize, const int* valid, const size_t validSize, const double lr);

// Init transition matrices and apply their corresponding random error in the data
void mkNetInitMatrices(MarkovNetwork* net);

void mkNetUpdateWeights(MarkovNetwork* net, const double lr, const size_t id, bool correct);
void mkNetNormStd(MarkovNetwork* net);
void mkNetNormSoftmax(MarkovNetwork* net, double temperature);

void mkNetSetLastState(MarkovNetwork* net, const int* lastState);
void mkNetPredict(MarkovNetwork* net, const size_t steps, int* predOut, double* confOut);

// Export Network Graph to DOT format (graph visualization tool)
void mkNetExport(const MarkovNetwork* net, const char* file);
/* -------------------------------------------------------------------------- */

/* -------------------------- USEFUL ERROR FUNCTIONS -------------------------- */
void randomBinarySwap(size_t nodeId, const int* data, int* out, size_t n, double errFactor);
void binarySegmentNoise(size_t nodeId, const int* data, int* out, size_t n, double errFactor);
/* ---------------------------------------------------------------------------- */

#endif //MARKOVNETWORK_H
