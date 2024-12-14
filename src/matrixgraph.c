#include "matrixgraph.h"

#include <stdlib.h>

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
        MatrixFreeTransMatrix(&(*node)->matrix);
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

/* ----------------------------- MATRIX EDGE ----------------------------- */
MatrixGraphEdge* mxEdgeInit(MatrixNode* orig, MatrixNode* dest, double weight) {
    MatrixGraphEdge* edge = malloc(sizeof(MatrixGraphEdge));
    if (!edge) {
        LOG_ERROR("malloc failed for MatrixGraphEdge* edge");
        return NULL;
    }

    edge->orig = orig;
    edge->dest = dest;
    edge->weight = weight;
    edge->next = NULL;

    return edge;
}

void mxEdgeFree(MatrixGraphEdge** edge) {
    if (!edge || !(*edge))
        return;

    // Dont free nodes because it may be shared
    free(*edge);
    *edge = NULL;
}

void mxEdgeEnds(const MatrixGraphEdge* edge, MatrixNode** orig, MatrixNode** dest) {
    if (!edge)
        return;
    if (orig) *orig = edge->orig;
    if (dest) *dest = edge->dest;
}

double mxEdgeWeight(const MatrixGraphEdge* edge) {
    if (!edge)
        return 0.0;
    return edge->weight;
}
/* ----------------------------------------------------------------------- */

/* ----------------------------- MATRIX GRAPH ----------------------------- */
MatrixGraph* mxGraphInit() {
    MatrixGraph* graph = malloc(sizeof(MatrixGraph));
    if (!graph) {
        LOG_ERROR("malloc failed for graph");
        return NULL;
    }

    // Fist allocate GRAPH_BUCKET_SIZE for the array of edges, and reallocate it if needed later
    graph->edges = calloc(GRAPH_BUCKET_SIZE, sizeof(MatrixGraphEdge*));
    graph->_memBuckets = 1;

    graph->nEdges = 0;
    graph->nNodes = 0;

    return graph;
}

void mxGraphFree(MatrixGraph** graph) {
    if (!graph || !(*graph))
        return;

    if ((*graph)->edges) {
        // First free every node
        // the nodes are the origin of every edges[i]
        for (size_t e = 0; e < (*graph)->nNodes; e++) {
            if ((*graph)->edges[e] && (*graph)->edges[e]->orig)
                mxNodeFree(&(*graph)->edges[e]->orig);
        }

        // Deep free every edge
        MatrixGraphEdge* curr = (*graph)->edges[0];
        while (curr) {
            MatrixGraphEdge* temp = curr->next;
            mxEdgeFree(&curr);
            curr = temp;
        }
        free((*graph)->edges);
    }

    // finally free the graph pointer
    free(*graph);
    *graph = NULL;
}

bool _mxGraphIncreaseBucket(MatrixGraph* graph) {
    graph->_memBuckets++;
    MatrixGraphEdge** newBucket = realloc(graph->edges, graph->_memBuckets * sizeof(MatrixGraphEdge*));
    if (!newBucket) {
        LOG_ERROR("Failed to allocate bucket with increased size for graph");
        return false;
    }
    graph->edges = newBucket;
    return true;
}

MatrixNode* mxGraphAddNode(MatrixGraph* graph, MatrixNode* node) {
    if (!graph || !node)
        return NULL;

    // Adjust node ID to be the newest ID
    node->id = graph->nNodes;

    // Check if need to increase bucket for adding node
    if (graph->nNodes+1 > graph->_memBuckets * GRAPH_BUCKET_SIZE) {
        if (!_mxGraphIncreaseBucket(graph)) {
            LOG_ERROR("Failed to increase bucket size. Not adding Node to graph");
            return NULL;
        }
    }

    // Insert node as an edge with NULL destiny and 0 weight
    graph->edges[graph->nNodes] = mxEdgeInit(node, NULL, 0.0);
    graph->nNodes++;
    return node;
}

MatrixNode* mxGraphAddTransMatrix(MatrixGraph* graph, TransitionMatrix* tm) {
    if (!graph || !tm)
        return NULL;

    // Create node with this matrix and latest id
    MatrixNode* node = mxNodeInit(graph->nNodes, tm);
    if (mxGraphAddNode(graph, node) == NULL) {
        LOG_ERROR("Failed to add transition matrix to graph");
        mxNodeFree(&node);
    }
    return node;
}

MatrixNode* mxGraphGetNode(const MatrixGraph* graph, size_t id) {
    if (!graph || id >= graph->nNodes)
        return NULL;
    return graph->edges[id]->orig;
}

bool mxGraphHasNode(const MatrixGraph* graph, const MatrixNode* node) {
    return (graph != NULL && node->id <= graph->nNodes);
}

MatrixGraphEdge* mxGraphAddEdge(MatrixGraph* graph, MatrixNode* orig, MatrixNode* dest, double weight) {
    if (!graph)
        return NULL;

    if (!mxGraphHasNode(graph, orig) || !mxGraphHasNode(graph, dest))
        return NULL;

    size_t origID = mxNodeId(orig);
    // walk to orig edges list to get to the last one
    MatrixGraphEdge* edge = graph->edges[origID];
    while (edge->next)
        edge = edge->next;
    edge->next = mxEdgeInit(orig, dest, weight);
    graph->nEdges++;
    return edge->next;
}

void mxGraphNodes(const MatrixGraph* graph, MatrixNode** outNodes) {
    if (!graph || !graph->edges || !outNodes)
        return;

    for (size_t i = 0; i < graph->nNodes; i++)
        outNodes[i] = graph->edges[i]->orig;
}

void mxGraphEdges(const MatrixGraph* graph, MatrixGraphEdge** outEdges) {
    if (!graph || !graph->edges || !outEdges)
        return;

    size_t count = 0;
    for (size_t i = 0; i < graph->nNodes; i++) {
        MatrixGraphEdge* edge = graph->edges[i];
        while (edge) {
            outEdges[count] = edge;
            count++;
            edge = edge->next;
        }
    }
}
