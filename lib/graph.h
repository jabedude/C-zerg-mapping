#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "tree.h"
#include "zerg.h"

// GRAPH[i][j] = 1 if i and j are adjacent
// GRAPH[i][j] = 0 if i and j are NOT adjacent

typedef struct _graph {
    int verts;
    int max_rems;
    int edges;
    Node **nod_list;
    double **mat; //2-d matrix
} Graph_t;

#define MIN_DIST 1.25000 * 0.91440
#define MAX_DIST 15.00000

Graph_t *mkgraph(unsigned int vertices);
void rmgraph(Graph_t *g);
void initgraph(Graph_t *g, Node *root);
void printgraph(const Graph_t *g);
bool isconn(const Graph_t *g);
void rmvert(Graph_t *g, int ind);
void fixgraph(Graph_t *g);

#endif
