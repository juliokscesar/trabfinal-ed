#include "markovgraph.h"

/* ----------------------------- MARKOV NODE ----------------------------- */
MarkovNode* mkNodeInit(const size_t id, TransitionMatrix* matrix) {
    MarkovNode* node = malloc(sizeof(MarkovNode));
    if (!node) {
        LOG_ERROR("malloc error for node in mkNodeInit");
        return NULL;
    }

    node->id = id;
    node->matrix = matrix;

    return node;
}

void mkNodeFree(MarkovNode** node) {
    if (!node || !(*node))
        return;

    if ((*node)->matrix)
        markovFreeTransMatrix(&(*node)->matrix);
    free(*node);
    *node = NULL;
}

size_t mkNodeId(const MarkovNode* node) {
    if (!node)
        return 0;
    return node->id;
}

TransitionMatrix* mkNodeMatrix(const MarkovNode* node) {
    if (!node)
        return NULL;
    return node->matrix;
}
/* ----------------------------------------------------------------------- */

/* ----------------------------- MARKOV EDGE ----------------------------- */
MarkovGraphEdge* mkEdgeInit(MarkovNode* orig, MarkovNode* dest, double weight) {
    MarkovGraphEdge* edge = malloc(sizeof(MarkovGraphEdge));
    if (!edge) {
        LOG_ERROR("malloc failed for MarkovGraphEdge* edge");
        return NULL;
    }

    edge->orig = orig;
    edge->dest = dest;
    edge->weight = weight;
    edge->next = NULL;

    return edge;
}

void mkEdgeFree(MarkovGraphEdge** edge) {
    if (!edge || !(*edge))
        return;

    // Dont free nodes because it may be shared
    free(*edge);
    *edge = NULL;
}

void mkEdgeEnds(const MarkovGraphEdge* edge, MarkovNode** orig, MarkovNode** dest) {
    if (!edge)
        return;
    if (orig) *orig = edge->orig;
    if (dest) *dest = edge->dest;
}

double mkEdgeWeight(const MarkovGraphEdge* edge) {
    if (!edge)
        return 0.0;
    return edge->weight;
}
/* ----------------------------------------------------------------------- */

/* ----------------------------- MARKOV GRAPH ----------------------------- */
MarkovGraph* mkGraphInit() {
    MarkovGraph* graph = malloc(sizeof(MarkovGraph));
    if (!graph) {
        LOG_ERROR("malloc failed for graph");
        return NULL;
    }

    // Fist allocate GRAPH_BUCKET_SIZE for the array of edges, and reallocate it if needed later
    graph->edges = calloc(GRAPH_BUCKET_SIZE, sizeof(MarkovGraphEdge*));
    graph->_memBuckets = 1;

    graph->nEdges = 0;
    graph->nNodes = 0;

    return graph;
}

void mkGraphFree(MarkovGraph** graph) {
    if (!graph || !(*graph))
        return;

    if ((*graph)->edges) {
        // First free every node
        // the nodes are the origin of every edges[i]
        for (size_t e = 0; e < (*graph)->nNodes; e++) {
            if ((*graph)->edges[e] && (*graph)->edges[e]->orig)
                mkNodeFree(&(*graph)->edges[e]->orig);
        }

        // Deep free every edge
        MarkovGraphEdge* curr = (*graph)->edges[0];
        while (curr) {
            MarkovGraphEdge* temp = curr->next;
            mkEdgeFree(&curr);
            curr = temp;
        }
        free((*graph)->edges);
    }

    // finally free the graph pointer
    free(*graph);
    *graph = NULL;
}

bool _mkGraphIncreaseBucket(MarkovGraph* graph) {
    graph->_memBuckets++;
    MarkovGraphEdge** newBucket = realloc(graph->edges, graph->_memBuckets * sizeof(MarkovGraphEdge*));
    if (!newBucket) {
        LOG_ERROR("Failed to allocate bucket with increased size for graph");
        return false;
    }
    graph->edges = newBucket;
    return true;
}

MarkovNode* mkGraphAddNode(MarkovGraph* graph, MarkovNode* node) {
    if (!graph || !node)
        return NULL;

    // Adjust node ID to be the newest ID
    node->id = graph->nNodes;

    // Check if need to increase bucket for adding node
    if (graph->nNodes+1 > graph->_memBuckets * GRAPH_BUCKET_SIZE) {
        if (!_mkGraphIncreaseBucket(graph)) {
            LOG_ERROR("Failed to increase bucket size. Not adding Node to graph");
            return NULL;
        }
    }

    // Insert node as an edge with NULL destiny and 0 weight
    graph->edges[graph->nNodes] = mkEdgeInit(node, NULL, 0.0);
    graph->nNodes++;
    return node;
}

MarkovNode* mkGraphAddTransMatrix(MarkovGraph* graph, TransitionMatrix* tm) {
    if (!graph || !tm)
        return NULL;

    // Create node with this matrix and latest id
    MarkovNode* node = mkNodeInit(graph->nNodes, tm);
    if (mkGraphAddNode(graph, node) == NULL) {
        LOG_ERROR("Failed to add transition matrix to graph");
        mkNodeFree(&node);
    }
    return node;
}

MarkovNode* mkGraphGetNode(const MarkovGraph* graph, size_t id) {
    if (!graph || id >= graph->nNodes)
        return NULL;
    return graph->edges[id]->orig;
}

bool mkGraphHasNode(const MarkovGraph* graph, const MarkovNode* node) {
    return (graph != NULL && node->id <= graph->nNodes);
}

MarkovGraphEdge* mkGraphAddEdge(MarkovGraph* graph, MarkovNode* orig, MarkovNode* dest, double weight) {
    if (!graph)
        return NULL;

    if (!mkGraphHasNode(graph, orig) || !mkGraphHasNode(graph, dest))
        return NULL;

    size_t origID = mkNodeId(orig);
    // walk to orig edges list to get to the last one
    MarkovGraphEdge* edge = graph->edges[origID];
    while (edge->next)
        edge = edge->next;
    edge->next = mkEdgeInit(orig, dest, weight);
    graph->nEdges++;
    return edge->next;
}

void mkGraphNodes(const MarkovGraph* graph, MarkovNode** outNodes) {
    if (!graph || !graph->edges || !outNodes)
        return;

    for (size_t i = 0; i < graph->nNodes; i++)
        outNodes[i] = graph->edges[i]->orig;
}

void mkGraphEdges(const MarkovGraph* graph, MarkovGraphEdge** outEdges) {
    if (!graph || !graph->edges || !outEdges)
        return;

    size_t count = 0;
    for (size_t i = 0; i < graph->nNodes; i++) {
        MarkovGraphEdge* edge = graph->edges[i];
        while (edge) {
            outEdges[count] = edge;
            count++;
            edge = edge->next;
        }
    }
}

