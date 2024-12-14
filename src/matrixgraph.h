#ifndef MATRIXGRAPH_H
#define MATRIXGRAPH_H

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

/* ----------------------------- MATRIX EDGE ----------------------------- */
// MatrixEdge represents each directed connection in the graph (orig->dest) with an
// associated weight. It's also a node of a linked list, so it has a pointer to a 'next' edge
typedef struct edge {
    MatrixNode* orig;
    MatrixNode* dest;
    double weight;
    struct edge* next;
} MatrixGraphEdge;

MatrixGraphEdge* mxEdgeInit(MatrixNode* orig, MatrixNode* dest, double weight);
void mxEdgeFree(MatrixGraphEdge** edge);
void mxEdgeEnds(const MatrixGraphEdge* edge, MatrixNode** orig, MatrixNode** dest);
double mxEdgeWeight(const MatrixGraphEdge* edge);
/* ----------------------------------------------------------------------- */

/* ----------------------------- MATRIX GRAPH ----------------------------- */
// MatrixGraph is the graph containing all nodes with different TransitionMatrix
// this allows us to join multiple transition matrices and combine their results
// by associating a weight to their answer, and using backpropagation we can adjust
// those weights.
#define GRAPH_BUCKET_SIZE 100
typedef struct {
    MatrixGraphEdge** edges;
    size_t nNodes;
    size_t nEdges;

    size_t _memBuckets;
} MatrixGraph;

MatrixGraph* mxGraphInit();
void mxGraphFree(MatrixGraph** graph);
MatrixNode* mxGraphAddNode(MatrixGraph* graph, MatrixNode* node);
MatrixNode* mxGraphAddTransMatrix(MatrixGraph* graph, TransitionMatrix* tm);
MatrixNode* mxGraphGetNode(const MatrixGraph* graph, size_t id);
bool mxGraphHasNode(const MatrixGraph* graph, const MatrixNode* node);
MatrixGraphEdge* mxGraphAddEdge(MatrixGraph* graph, MatrixNode* orig, MatrixNode* dest, double weight);
void mxGraphNodes(const MatrixGraph* graph, MatrixNode** outNodes);
void mxGraphEdges(const MatrixGraph* graph, MatrixGraphEdge** outEdges);
/* ------------------------------------------------------------------------ */

#endif //MATRIXGRAPH_H
