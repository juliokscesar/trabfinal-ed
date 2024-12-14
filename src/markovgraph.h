#ifndef MARKOVGRAPH_H
#define MARKOVGRAPH_H

#include <stdlib.h>

#include "markov.h"
#include "logging.h"

/// Graph implementation with Transition Matrices as nodes

/* ----------------------------- MARKOV NODE ----------------------------- */
// MarkovNode represents each node in the graph containing one possible state
typedef struct {
    size_t id;
    uint order;
    int* state;
} MarkovNode;

MarkovNode* mkNodeInit(const size_t id, const uint order, int* state);
void mkNodeFree(MarkovNode** node);
size_t mkNodeId(const MarkovNode* node);
int* mkNodeState(const MarkovNode* node);
/* ----------------------------------------------------------------------- */

/* ----------------------------- MARKOV EDGE ----------------------------- */
// MarkovEdge represents each directed connection in the graph (orig->dest) with an
// associated weight. It's also a node of a linked list, so it has a pointer to a 'next' edge
// The associated weight is the probability of the state 'orig' to go to the state 'dest'
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
// MarkovGraph is the graph containing all nodes with different states
// Every node is connected to a different state, and the weight associated with
// that edge is the probability.
#define GRAPH_BUCKET_SIZE 100
typedef struct {
    MarkovGraphEdge** edges;
    size_t nNodes;
    size_t nEdges;

    uint order;
    int* vals;
    size_t nVals;
} MarkovGraph;

MarkovGraph* mkGraphInit(const MarkovState* states);
void mkGraphFree(MarkovGraph** graph);
void mkGraphBuildTransitions(MarkovGraph* graph, const TransitionMatrix* tm);
MarkovNode* mkGraphGetNode(const MarkovGraph* graph, size_t id);
bool mkGraphHasNode(const MarkovGraph* graph, const MarkovNode* node);
lli mkGraphIdState(const MarkovGraph* graph, const int* state);
MarkovGraphEdge* mkGraphAddEdge(MarkovGraph* graph, MarkovNode* orig, MarkovNode* dest, double weight);
void mkGraphNodes(const MarkovGraph* graph, MarkovNode** outNodes);
void mkGraphEdges(const MarkovGraph* graph, MarkovGraphEdge** outEdges);

// Export graph to DOT format (graph visualization tool)
void mkGraphExport(const MarkovGraph* graph, const char* file);
/* ------------------------------------------------------------------------ */

#endif // MARKOVGRAPH_H