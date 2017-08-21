#ifndef GRAPH_H
#define GRAPH_H

// GRAPH[i][j] = 1 if i and j are adjacent
// GRAPH[i][j] = 0 if i and j are NOT adjacent

typedef struct _graph {
    int verts;
    int edge;
    int **mat; //2-d matrix
} Graph_t;

Graph_t *mkgraph(int vertices);
void rmgraph(Graph_t *g);
void initgraph(Graph_t *g);
double havdist(ZergBlock_t *zba, ZergBlock_t *zbb);

#endif
