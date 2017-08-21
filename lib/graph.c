#include <math.h>

#include "graph.h"
#include "zerg.h"

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

double havdist(ZergBlock_t *zba, ZergBlock_t *zbb)
{
    //double long1 = zerg1->longitude;
    //double lat1 = zerg1->latitude;
    //double alt1 = zerg1->altitude;

    //double long2 = zerg2->longitude;
    //double lat2 = zerg2->latitude;
    //double alt2 = zerg2->altitude;

    // TODO: refactor this stuff

    double dlog = TO_RAD * ((zbb->z_long - zba->z_long) / 2);
    double dlat = TO_RAD * ((zbb->z_lat - zba->z_lat) / 2);

    double lon_a = TO_RAD * zba->z_long;
    double lon_b = TO_RAD * zbb->z_long;
    double lat_a = TO_RAD * zba->z_lat;
    double lat_b = TO_RAD * zbb->z_lat;

    double A = 0.5 * (1 - cos(2 * dlat));
    double B = cos(lat_a) * cos(lat_b);
    double C = 0.5 * (1 - cos(2 * dlog));

    double linearD = 2 * R * asin(sqrt(A + B * C));

    double heightD = (zba->z_alt - zbb->z_alt) / 1000;

    return 1000 * sqrt(linearD * linearD + heightD * heightD);
}
