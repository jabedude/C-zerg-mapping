#ifndef TREE_H
#define TREE_H

#include "zerg.h"

/*********STRUCTURES********/
typedef struct node {
    ZergBlock_t *zergblk;
    struct node *left;
    struct node *right;
} Node;

/**********DEFINES**********/
#define UPDATE_SUC 1
#define UPDATE_INV 0
#define UPDATE_NOT -1

/*********FUNCTIONS*********/
ZergBlock_t *mkblk(void); //TODO: look into macro-ing or inlining this
Node *mknode(void);
void printnode(const Node *n);
void rmtree(Node *root);
void rmlist(Node *head);
Node *trtol(Node *root);
void listprint(Node *head);
void ordprint(Node *root);
void nadd(Node *root, Ticker *company);
int updtree(Node *root, Ticker *term);

#endif
