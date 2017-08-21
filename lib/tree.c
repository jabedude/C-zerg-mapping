#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "zerg.h"
#include "tree.h"

static double ieee_convert64(uint64_t num)
{
    /* All credit to droberts */
    uint8_t sign;
    uint16_t exponent;
    uint64_t mantisa;
    double result = 0;

    if (num == 0)
        return 0L;

    sign = num >> 63;
    exponent = (num >> 52 & 0x7FF) - 1023;
    mantisa = num & 0xFFFFFFFFFFFFF;
    result = (mantisa *pow(2, -52)) + 1;
    result *= pow(1, sign) * pow(2, exponent);
    return result;
}

static double ieee_convert32(uint32_t num)
{
    /* All credit to droberts */
    uint8_t sign, exponent;
    uint32_t mantisa;
    double result = 0;

    sign = num >> 31;
    exponent = (num >> 23 & 0xFF) - 127;
    mantisa = num & 0x7FFFFF;

    result = (mantisa *pow(2, -23)) + 1;
    result *= pow(1, sign) * pow(2, exponent);

    return result;
}

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

ZergBlock_t *mkblk(void)
{
    ZergBlock_t *zb = malloc(sizeof(ZergBlock_t));
    return zb;
}

Node *mknode(void)
{
    Node *n      = malloc(sizeof(Node));
    n->zergblk   = NULL;
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

void nadd(Node *root, ZergBlock_t *zb)
{
    if (root->zergblk) { //Non-empty tree
        Node *n = root;
        Node *m = root;
        while (n) {
            m = n;
            if (n->zergblk->z_id < zb->z_id)
                n = n->right;
            else if (n->zergblk->z_id > zb->z_id)
                n = n->left;
            else {          //Already in BST
                n->zergblk->z_hp[0] = zb->z_hp[0];
                n->zergblk->z_hp[1] = zb->z_hp[1];
                n->zergblk->z_hp[2] = zb->z_hp[2];
                n->zergblk->z_long = zb->z_long;
                n->zergblk->z_lat  = zb->z_lat;
                n->zergblk->z_alt  = zb->z_alt;
                free(zb);
                return;
            }
        }
        Node *tmp = mknode();
        tmp->zergblk = zb;
        if (m->zergblk->z_id < zb->z_id)
            m->right = tmp;
        else if (m->zergblk->z_id > zb->z_id)
            m->left = tmp;
    } else {            //Empty tree
        root->zergblk = zb;
    }
}

void printnode(const Node *n)
{
    double longitude, latitude;

    //longitude = ieee_convert64(ntoh64(n->zergblk->z_long));
    longitude = bin64(n->zergblk->z_long);
    //latitude = ieee_convert64(ntoh64(n->zergblk->z_lat));
    latitude = bin64(n->zergblk->z_lat);
    printf("Source: %d\tHP: %d/%u\tLong: %f deg\tLat: %f\tAlt: %6.4f\n",
           ntohs(n->zergblk->z_id),
           NTOH3(n->zergblk->z_hp),
           NTOH3(n->zergblk->z_maxhp),
           longitude,
           latitude,
           ieee_convert32(ntohl(n->zergblk->z_alt)));
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
        free(head->zergblk);
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
        //printf("freeing node: %d\n", root->zergblk->z_id);
        free(root->zergblk);
        free(root);
    }
}

int updtree(Node *root, ZergBlock_t *zb)
{
    Node *tmp = root;
    while (tmp) {
        if (tmp->zergblk->z_id < zb->z_id)
            tmp = tmp->right;
        else if (tmp->zergblk->z_id > zb->z_id)
            tmp = tmp->left;
        else {
            if (0)
                return UPDATE_INV;
            tmp->zergblk->z_hp[0] = zb->z_hp[0];
            tmp->zergblk->z_hp[1] = zb->z_hp[1];
            tmp->zergblk->z_hp[2] = zb->z_hp[2];
            return UPDATE_SUC;
        }
    }
    return UPDATE_NOT;
}
