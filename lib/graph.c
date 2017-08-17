#include "graph.h"

Graph_t *mkgraph(int vertices)
{
    Graph_t *g = malloc(sizeof(Graph_t));
    g->vert = vertices;
    g->mat = malloc(vertices * sizeof(int*));

    for (int i = 0; i < vertices; i++)
        g->mat[i] = malloc(vertices * sizeof(int));

    return g;
}
