#include "markovgraph.h"

#include <string.h>

#include "utils.h"

/* ----------------------------- MARKOV NODE ----------------------------- */
MarkovNode* mkNodeInit(const size_t id, const uint order, int* state) {
    MarkovNode* node = malloc(sizeof(MarkovNode));
    if (!node) {
        LOG_ERROR("malloc error for node in mkNodeInit");
        return NULL;
    }

    node->id = id;
    node->order = order;
    // don't copy state
    node->state = state;

    return node;
}

void mkNodeFree(MarkovNode** node) {
    if (!node || !(*node))
        return;

    free(*node);
    *node = NULL;
}

size_t mkNodeId(const MarkovNode* node) {
    if (!node)
        return 0;
    return node->id;
}

int* mkNodeState(const MarkovNode* node) {
    if (!node)
        return NULL;
    return node->state;
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
MarkovGraph* mkGraphInit(const MarkovState* states) {
    if (!states)
        return NULL;

    MarkovGraph* graph = malloc(sizeof(MarkovGraph));
    if (!graph) {
        LOG_ERROR("malloc failed for graph");
        return NULL;
    }

    graph->order = states->order;
    graph->vals = states->vals;
    graph->nVals = states->nVals;

    // The number of nodes is the amount of states
    graph->nNodes = states->nStates;
    graph->edges = malloc(sizeof(MarkovGraphEdge*) * graph->nNodes);
    if (!graph->edges) {
        LOG_ERROR("malloc failed for graph->edges");
        free(graph);
        return NULL;
    }

    // initialize the nodes in the order of the states
    for (size_t i = 0; i < states->nStates; i++) {
        MarkovNode* stateNode = mkNodeInit(i, graph->order, states->states[i]);
        if (!stateNode) {
            LOG_ERROR("mkNodeInit failed for stateNode");
            // Cleanup already allocated edges
            for (size_t j = 0; j < i; j++) {
                mkNodeFree(&(graph->edges[j]->orig));
                mkEdgeFree(&(graph->edges[j]));
            }
            mkNodeFree(&stateNode);
            free(graph->edges);
            free(graph);
            return NULL;
        }

        graph->edges[i] = mkEdgeInit(stateNode, NULL, 0.0);
        if (!graph->edges[i]) {
            LOG_ERROR("mkEdgeInit failed for edge");
            mkNodeFree(&stateNode);
            for (size_t j = 0; j < i; j++) {
                mkNodeFree(&(graph->edges[i]->orig));
                mkEdgeFree(&(graph->edges[j]));
            }
            free(graph->edges);
            free(graph);
            return NULL;
        }
    }
    graph->nEdges = 0;
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

            // Deep free every edge of this node
            MarkovGraphEdge* curr = (*graph)->edges[e];
            while (curr) {
                MarkovGraphEdge* temp = curr->next;
                mkEdgeFree(&curr);
                curr = temp;
            }
        }

        free((*graph)->edges);
    }

    // finally free the graph pointer
    free(*graph);
    *graph = NULL;
}

void mkGraphBuildTransitions(MarkovGraph* graph, const TransitionMatrix* tm) {
    if (!graph || !tm)
        return;

    // For every state, we have the probability of the next value being 1 or 0
    // so the next state is the current state with the last value replaced by this new one
    // and the past values translated to the left
    int* nextState = malloc(sizeof(int) * graph->order);
    if (!nextState) {
        LOG_ERROR("malloc failed for tempState, unable to initialize graph probabilities.");
        return;
    }
    for (size_t stateID = 0; stateID < graph->nNodes; stateID++) {
        MarkovNode* stateNode = graph->edges[stateID]->orig;

        // copy only last two values of state
        memcpy(nextState, stateNode->state+1, sizeof(int) * (graph->order - 1));
        for (size_t valID = 0; valID < graph->nVals; valID++) {
            // set next value for next state
            nextState[graph->order - 1] = graph->vals[valID];
            const lli nextID = mkGraphIdState(graph, nextState);
            if (nextID == -1) {
                LOG_ERROR("Unidentified state: ");
                printArr_i(nextState, graph->order);
                continue;
            }
            MarkovNode* nextNode = mkGraphGetNode(graph, nextID);

            // Then add an edge for this transition with the probability from the matrix
            MarkovGraphEdge* edge = mkGraphAddEdge(graph, stateNode, nextNode, tm->probs[stateID][valID]);
            if (!edge) {
                LOG_ERROR("Unable to add edge for state transition");
                fprintf(stderr, "From ID %ld to ID %ld\n", stateID, nextNode->id);
            }
        }
    }
    free(nextState);
}

MarkovNode* mkGraphGetNode(const MarkovGraph* graph, size_t id) {
    if (!graph || id >= graph->nNodes)
        return NULL;
    return graph->edges[id]->orig;
}

bool mkGraphHasNode(const MarkovGraph* graph, const MarkovNode* node) {
    return (graph != NULL && node->id <= graph->nNodes);
}

lli mkGraphIdState(const MarkovGraph* graph, const int* state) {
    if (!graph || !state)
        return -1;

    for (size_t id = 0; id < graph->nNodes; id++) {
        const MarkovNode* stateNode = graph->edges[id]->orig;
        if (memcmp(stateNode->state, state, sizeof(int) * graph->order) == 0)
            return id;
    }

    return -1;
}

MarkovGraphEdge* mkGraphAddEdge(MarkovGraph* graph, MarkovNode* orig, MarkovNode* dest, double weight) {
    if (!graph)
        return NULL;

    if (!mkGraphHasNode(graph, orig) || !mkGraphHasNode(graph, dest))
        return NULL;

    graph->nEdges++;
    size_t origID = mkNodeId(orig);
    // walk through orig edges list to get to the last one
    MarkovGraphEdge* edge = graph->edges[origID];
    // Replace if this is the first one (edge->dest is NULL)
    if (!edge->dest) {
        edge->dest = dest;
        edge->weight = weight;
        return edge;
    }

    while (edge->next)
        edge = edge->next;
    edge->next = mkEdgeInit(orig, dest, weight);
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

MarkovGraphEdge** mkGraphNodePaths(const MarkovGraph* graph, const MarkovNode* node, size_t* count) {
    if (!graph || !node || !count)
        return NULL;

    // Pre-allocate the maximum number of edges
    MarkovGraphEdge** paths = calloc(graph->nEdges, sizeof(MarkovGraphEdge*));
    if (!paths) {
        LOG_ERROR("Failed to allocate memory for node paths in mkGraphNodePaths, nEdges=");
        printf("%lu\n", graph->nEdges);
        return NULL;
    }

    *count = 0;
    MarkovGraphEdge* edge = graph->edges[node->id];
    while (edge) {
        paths[*count] = edge;
        (*count)++;
        edge = edge->next;
    }

    // Shrink memory usage if possible
    if (*count < graph->nEdges) {
        MarkovGraphEdge** temp = realloc(paths, sizeof(MarkovGraphEdge*) * (*count));
        if (!temp) {
            LOG_WARNING("Failed to reallocate graph node paths with less memory");
            return paths;
        }
        paths = temp;
    }

    return paths;
}

MarkovNode** mkGraphNeighbors(const MarkovGraph* graph, const MarkovNode* node, size_t* count) {
    if (!graph || !node || !count)
        return NULL;
    if (!mkGraphHasNode(graph, node))
        return NULL;

    // Pre-allocate the maximum number of edges
    MarkovNode** neighbors = calloc(graph->nNodes, sizeof(MarkovNode*));
    *count = 0;
    MarkovGraphEdge* edge = graph->edges[node->id];
    while (edge) {
        neighbors[*count] = edge->dest;
        (*count)++;
        edge = edge->next;
    }

    // Shrink memory block if used size is less than allocated
    if (*count < graph->nNodes) {
        MarkovNode** temp = realloc(neighbors, sizeof(MarkovNode*) * (*count));
        if (!temp) {
            LOG_WARNING("Failed to reallocate neighbors array with less size. Sending as it is");
            return neighbors;
        }
        neighbors = temp;
    }

    return neighbors;
}

size_t* mkGraphFindDisconnected(const MarkovGraph* graph, size_t* count) {
    if (!graph)
        return NULL;

    bool* visited = calloc(graph->nNodes, sizeof(bool));
    // BFS in graph to find all visited nodes
    // Only mark as visited those nodes that are destinies from other states,
    // and whose paths probabilities (weights) are bigger than 0.0
    for (size_t i = 0; i < graph->nNodes; i++) {
        MarkovGraphEdge* edge = graph->edges[i];
        while (edge) {
            if (edge->dest && (edge->weight > 1e-3))
                visited[edge->dest->id] = true;
            edge = edge->next;
        }
    }

    // In the end, the disconnected nodes will be those that weren't visited
    size_t* disconnected = malloc(graph->nNodes * sizeof(size_t));
    for (size_t i = 0; i < graph->nNodes; i++) {
        if (!visited[i]) {
            disconnected[*count] = i;
            (*count)++;
        }
    }

    if (*count == 0) {
        free(visited);
        return NULL;
    }

    // shrink memory if possible
    if (*count < graph->nNodes) {
        size_t* temp = realloc(disconnected, sizeof(size_t) * (*count));
        if (!temp)
            LOG_WARNING("Couldn't resize disconnected array with less memory in mkGraphFindDisconnected");
        else
            disconnected = temp;
    }

    free(visited);
    return disconnected;
}

void mkGraphRandWalk(const MarkovGraph* graph, const int* lastState, const size_t steps, int* stopOut,
                     double* probsOut) {
    // Random walk on the Markov Graph will provide a way to predict next states
    if (!graph || !lastState || !stopOut)
        return;

    lli lastID = mkGraphIdState(graph, lastState);
    if (lastID == -1) {
        LOG_ERROR("Couldn't id last state in mkGraphRandWalk: ");
        printArr_i(lastState, graph->order);
        return;
    }

    MarkovNode* pos = mkGraphGetNode(graph, lastID);
    for (size_t step = 0; step < steps; step++) {
        size_t nPaths = 0;
        MarkovGraphEdge** paths = mkGraphNodePaths(graph, pos, &nPaths);

        // choose path by cumulative probability
        double r = rand01_d();
        double cumProb = 0.0;
        for (size_t p = 0; p < nPaths; p++) {
            if (!paths[p]->dest)
                continue;
            cumProb += paths[p]->weight;
            if (r <= cumProb) {
                pos = paths[p]->dest;
                if (probsOut)
                    probsOut[step] = cumProb;
                break;
            }
        }

        // The predicted value will be the last value of the new position
        stopOut[step] = pos->state[pos->order - 1];

        free(paths);
    }
}

void mkGraphExport(const MarkovGraph* graph, const char* file) {
    if (!graph || !file)
        return;

    FILE* out = fopen(file, "w");
    if (!out) {
        LOG_ERROR("Unable to open file to export graph");
        return;
    }

    char stateStr[256] = {'\0'};
    char nextStr[256] = {'\0'};

    fprintf(out, "digraph G {\n");
    for (size_t i = 0; i < graph->nNodes; i++) {
        MarkovGraphEdge* edge = graph->edges[i];
        while (edge && edge->dest) {
            for (size_t s = 0; s < graph->order; s++) {
                sprintf(stateStr, "%s%d", stateStr, edge->orig->state[s]);
                sprintf(nextStr, "%s%d", nextStr, edge->dest->state[s]);
            }
            stateStr[graph->order] = '\0';
            nextStr[graph->order] = '\0';

            fprintf(out, "    \"%s\" -> \"%s\" [label=\"%.2f\"];\n", stateStr, nextStr, edge->weight);

            edge = edge->next;

            memset(stateStr, '\0', 256);
            memset(nextStr, '\0', 256);
        }
    }
    fprintf(out, "}\n");

    fclose(out);
    LOG_INFO("Graph exported to ");
    fprintf(stderr, "%s\n", file);
}
