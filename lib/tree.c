#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "tree.h"

ZergBlock_t *mkblk(void)
{
    ZergBlock_t *zb = calloc(1, sizeof(ZergBlock_t));
    return zb;
}

Node *mknode(void)
{
    Node *n      = malloc(sizeof(Node));
    n->zergblk   = mkblk();
    n->left      = NULL;
    n->right     = NULL;
    return n;
}

static Node *trtolhelp(Node *root)
{
    if (root == NULL)
        return root;

    if (root->left != NULL) {
        Node *left = trtolhelp(root->left);

        for (; left->right != NULL; left=left->right);

        left->right = root;

        root->left = left;
    }

    if (root->right != NULL) {
        Node *right = trtolhelp(root->right);

        for (; right->left != NULL; right = right->left);

        right->left = root;

        root->right = right;
    }

    return root;
}

Node *trtol(Node *root)
{
    /* http://www.geeksforgeeks.org/in-place-convert-a-given-binary-tree-to-doubly-linked-list/ */
    if (root == NULL)
        return root;

    root = trtolhelp(root);

    while (root->left != NULL)
        root = root->left;

    return root;
}

void nadd(Node *root, Ticker *company)
{
    if (root->tick->price) { //Non-empty tree
        Node *n = root;
        Node *m = root;
        while (n) {
            m = n;
            if (strcasecmp(n->tick->symb, company->symb) < 0)
                n = n->right;
            else if (strcasecmp(n->tick->symb, company->symb) > 0)
                n = n->left;
            else          //Already in BST
                return;
        }
        Node *tmp = mknode();
        tmp->tick = company;
        if (strcasecmp(m->tick->symb, company->symb) < 0)
            m->right = tmp;
        else if (strcasecmp(m->tick->symb, company->symb) > 0)
            m->left = tmp;
    } else {            //Empty tree
        root->tick = company;
    }
}

void printnode(const Node *n)
{
    double tmp = (double) n->tick->price / 100;
    printf("%s %.2lf %s\n", n->tick->symb, tmp, n->tick->name);
}

void listprint(Node *head)
{
    while (head) {
        printnode(head);
        head = head->right;
    }
}

void rmlist(Node *head)
{
    if (head) {
        rmlist(head->right);
        free(head->tick);
        free(head);
    }
}

void ordprint(Node *root)
{
    if (root) {
        ordprint(root->left);
        printnode(root);
        ordprint(root->right);
    }
    return;
}

void rmtree(Node *root)
{
    if (root) {
        rmtree(root->left);
        rmtree(root->right);
        free(root->tick);
        free(root);
    }
}

int updtree(Node *root, Ticker *term)
{
    Node *tmp = root;
    while (tmp) {
        if (strcasecmp(tmp->tick->symb, term->symb) < 0)
            tmp = tmp->right;
        else if (strcasecmp(tmp->tick->symb, term->symb) > 0)
            tmp = tmp->left;
        else {
            if ((tmp->tick->price + term->price < 1) || (tmp->tick->price + term->price > 1000000))
                return UPDATE_INV;
            tmp->tick->price += term->price;
            return UPDATE_SUC;
        }
    }
    return UPDATE_NOT;
}
