#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include "zerg.h"

// GRAPH[i][j] = 1 if i and j are adjacent
// GRAPH[i][j] = 0 if i and j are NOT adjacent

typedef struct _graph {
    int verts;
    int edge;
    double **mat; //2-d matrix
} Graph_t;

Graph_t *mkgraph(unsigned int vertices);
void rmgraph(Graph_t *g);
void initgraph(Graph_t *g, Node *root);
void printgraph(const Graph_t *g);

#endif
