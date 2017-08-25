#include <math.h>

#include "tree.h"
#include "graph.h"
#include "zerg.h"

static uint64_t ntoh64(uint64_t val)
{
    /* https://stackoverflow.com/a/2637138/5155574 */
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
    return (val << 32) | (val >> 32);
}

static double bin64(uint64_t num)
{
    union {
        uint64_t dec;
        double flt;
    } u_f;
    u_f.dec = ntoh64(num);
    return u_f.flt;
}

static double bin32(uint32_t num)
{
    union {
        uint32_t dec;
        double flt;
    } u_f;
    u_f.dec = ntohl(num);
    return u_f.flt;
}

static double dist(ZergBlock_t *zba, ZergBlock_t *zbb)
{
    double lat1 = bin64(zba->z_lat);
    double lon1 = bin64(zba->z_long);
    double alt1 = bin32(zba->z_alt);
    double lat2 = bin64(zbb->z_lat);
    double lon2 = bin64(zbb->z_long);
    double alt2 = bin32(zbb->z_alt);
    //the average circumference of earth; great circle radius
    const double r = 6372.8;

    //PI in radians
    const double PIByRad = 3.141592 / 180;

    double dlog = PIByRad * ((lon2 - lon1) / 2);
    double dlat = PIByRad * ((lat2 - lat1) / 2);

    lon1 *= PIByRad;
    lon2 *= PIByRad;
    lat1 *= PIByRad;
    lat2 *= PIByRad;

    //Identity sin(x)^2 = 0.5 * (1 - cos(2x))
    double a = 0.5 * (1 - cos(2 * dlat));
    double b = cos(lat1) * cos(lat2);
    double c = 0.5 * (1 - cos(2 * dlog));

    double linearD = 2 * r * asin(sqrt(a + b * c));

    double heightD = (alt1 - alt2) / 1000;

    return 1000 * sqrt(linearD * linearD + heightD * heightD);
}

static void _arr_frm_tr(const Node *root, Node **nod_list)
{
    static unsigned int ind = 0;

    if (root) {
        _arr_frm_tr(root->left, nod_list);
        nod_list[ind++] = (Node *) root;
        _arr_frm_tr(root->right, nod_list);
    }
    return;
}

void rmgraph(Graph_t *g)
{
    for (int i = 0; i < g->verts; i++) {
        double *dptr = g->mat[i];
        free(dptr);
    }
    free(g->mat);
    free(g->nod_list);
    free(g);
}

void printgraph(const Graph_t *g)
{
    for (int i = 0; i < g->verts; i++) {
        for (int j = 0; j < g->verts; j++) {
            printf("%.2f\t", g->mat[i][j]);
        }
        putchar('\n');
    }
}

Graph_t *mkgraph(unsigned int vertices)
{
    Graph_t *g = malloc(sizeof(Graph_t));
    g->edges = 0;
    g->max_rems = vertices;
    g->verts = vertices;
    g->nod_list = malloc(vertices * sizeof(Node*));
    g->mat = malloc(vertices * sizeof(double*));

    for (unsigned int i = 0; i < vertices; i++)
        g->mat[i] = malloc(vertices * sizeof(double));

    return g;
}

bool isconn(const Graph_t *g)
{
    /* Edge count */
    return g->edges >= (g->verts * 2);
}

void rmvert(Graph_t *g, int ind)
{
    static int v_dels = 0;

    /* Remove item */
    for (int i = ind; i < g->verts - 1; i++) {
        g->nod_list[i] = g->nod_list[i+1];
        g->mat[i] = g->mat[i+1];
    }
    --(g->verts);
    ++v_dels;

    for (int i = 0; i < g->verts; i++)
    for (int j = 0; j < g->verts; j++) {
        g->mat[i][j] = dist(g->nod_list[i]->zergblk, g->nod_list[j]->zergblk);
    }

    if (v_dels > (g->max_rems) / 2.0) { //Not going to remove more than half of the graph
        fprintf(stderr, "Removed more than half of zerg units\n");
        rmgraph(g);
        exit(1);
    }
}

void fixgraph(Graph_t *g)
{
    /* Neighbor count for each vertex */
    for (int i = 0; i < g->verts; i++) {
        int n_coun = 0;
        for (int j = 0; j < g->verts; j++) {
            if ((g->mat[i][j] < MAX_DIST) && (g->mat[i][j] > MIN_DIST))
                ++n_coun;
        }
        if (n_coun < 2) { //vertex has too few neighbors. bye bye!
            printf("REMOVING ZERG %d at IND %d\n", ntohs(g->nod_list[i]->zergblk->z_id), i);
            rmvert(g, i);
        }
    }
}

void initgraph(Graph_t *g, Node *root)
{
    for (int i = 0; i < g->verts; i++)
    for (int j = 0; j < g->verts; j++)
            g->mat[i][j] = 0;

    _arr_frm_tr(root, g->nod_list);
    for (size_t i = 0; i < nodecount(root); i++)
    for (size_t j = 0; j < nodecount(root); j++) {
        g->mat[i][j] = dist(g->nod_list[i]->zergblk, g->nod_list[j]->zergblk);
        if ((g->mat[i][j] < MAX_DIST) && (g->mat[i][j] > MIN_DIST))
            ++(g->edges);
    }
    return;
}

/*
1. neighbor count
2. reachability test via sets

*/
