#include "graph.h"

static double dist(double th1, double ph1, double th2, double ph2)
{   /* https://rosettacode.org/wiki/Haversine_formula#C */
#define R 6371
#define TO_RAD (3.1415926536 / 180)
    double dx, dy, dz;
    ph1 -= ph2;
    ph1 *= TO_RAD, th1 *= TO_RAD, th2 *= TO_RAD;

    dz = sin(th1) - sin(th2);
    dx = cos(ph1) * cos(th1) - cos(th2);
    dy = sin(ph1) * cos(th1);
    return asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * R;
}

Graph_t *mkgraph(int vertices)
{
    Graph_t *g = malloc(sizeof(Graph_t));
    g->verts = vertices;
    g->mat = malloc(vertices * sizeof(int*));

    for (int i = 0; i < vertices; i++)
        g->mat[i] = malloc(vertices * sizeof(int));

    return g;
}

void initgraph(Graph_t *g)
{
    for (int i = 0; i < g->verts; i++)
        for (int j = 0; j < g->verts; j++)
            g->mat[i][j] = 0;
    //TODO: determine adjacency here

    return;
}
