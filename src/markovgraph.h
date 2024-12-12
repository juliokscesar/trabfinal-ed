#ifndef MARKOVGRAPH_H
#define MARKOVGRAPH_H

#include <stdlib.h>

#include "markov.h"
#include "logging.h"

/// Graph implementation with Transition Matrices as nodes

/* ----------------------------- MARKOV NODE ----------------------------- */
// MarkovNode represents each node in the graph containing one TransitionMatrix
typedef struct {
    size_t id;
    TransitionMatrix* matrix;
} MarkovNode;

MarkovNode* mkNodeInit(const size_t id, TransitionMatrix* matrix);
void mkNodeFree(MarkovNode** node);
size_t mkNodeId(const MarkovNode* node);
TransitionMatrix* mkNodeMatrix(const MarkovNode* node);
/* ----------------------------------------------------------------------- */

/* ----------------------------- MARKOV EDGE ----------------------------- */
// MarkovEdge represents each directed connection in the graph (orig->dest) with an
// associated weight. It's also a node of a linked list, so it has a pointer to a 'next' edge
typedef struct edge {
    MarkovNode* orig;
    MarkovNode* dest;
    double weight;
    struct edge* next;
} MarkovGraphEdge;

MarkovGraphEdge* mkEdgeInit(MarkovNode* orig, MarkovNode* dest, double weight);
void mkEdgeFree(MarkovGraphEdge** edge);
void mkEdgeEnds(const MarkovGraphEdge* edge, MarkovNode** orig, MarkovNode** dest);
double mkEdgeWeight(const MarkovGraphEdge* edge);
/* ----------------------------------------------------------------------- */

/* ----------------------------- MARKOV GRAPH ----------------------------- */
// MarkovGraph is the graph containing all nodes with different TransitionMatrix
// this allows us to join multiple transition matrices and combine their results
// by associating a weight to their answer, and using backpropagation we can adjust
// those weights.
#define GRAPH_BUCKET_SIZE 100
typedef struct {
    MarkovGraphEdge** edges;
    size_t nNodes;
    size_t nEdges;

    size_t _memBuckets;
} MarkovGraph;

MarkovGraph* mkGraphInit();
void mkGraphFree(MarkovGraph** graph);
MarkovNode* mkGraphAddNode(MarkovGraph* graph, MarkovNode* node);
MarkovNode* mkGraphAddTransMatrix(MarkovGraph* graph, TransitionMatrix* tm);
MarkovNode* mkGraphGetNode(const MarkovGraph* graph, size_t id);
bool mkGraphHasNode(const MarkovGraph* graph, const MarkovNode* node);
MarkovGraphEdge* mkGraphAddEdge(MarkovGraph* graph, MarkovNode* orig, MarkovNode* dest, double weight);
void mkGraphNodes(const MarkovGraph* graph, MarkovNode** outNodes);
void mkGraphEdges(const MarkovGraph* graph, MarkovGraphEdge** outEdges);
/* ------------------------------------------------------------------------ */

#endif //MARKOVGRAPH_H