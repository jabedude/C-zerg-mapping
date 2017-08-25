#include "tree.h"

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
    if (num == 0)
        return num;

    union {
        uint32_t dec;
        double flt;
    } u_f;
    u_f.dec = ntohl(num);
    return u_f.flt;
}

static int _ntoh3(uint8_t* x)
{
    int y =((int) x[0] << 16) | ((int) (x[1]) << 8) | ((int) (x[2]));
    return y;
}

/*
 * function: mkblk
 * allocates memory for a zerg info block.
 *
 * return: pointer to allocated space for a zero'd out ZergBlock_t.
 */
ZergBlock_t *mkblk(void)
{
    ZergBlock_t *zb = malloc(sizeof(ZergBlock_t));
    memset(zb, 0, sizeof(ZergBlock_t));
    return zb;
}

/*
 * function: nodecount
 * return the number of nodes in a BST
 *
 * root: pointer to root of BST
 *
 * return: size_t number of nodes in the BST
 */
size_t nodecount(const Node *root)
{
    if (root == NULL)
        return 0;
    else {
        return 1 + nodecount(root->left) + nodecount(root->right);
    }
}

/*
 * function: mknode
 * returns a dynamic node for BST.
 */
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
    if (root == NULL)
        return root;

    root = trtolhelp(root);

    while (root->left != NULL)
        root = root->left;

    return root;
}

/*
 * function: nadd
 * adds nodes to BST. If node has a zerg id matching a node in tree,
 * node is updated with new values (coordinates, HP, etc.)
 *
 * root: root of BST we are adding to.
 * zb: zerg info block to try to add to BST.
 */
void nadd(Node *root, ZergBlock_t *zb)
{
#define MAX_ALT 7 * 0.0011364
    /* Check for valid lat/long values */
    double longitude = bin64(zb->z_long);
    double latitude = bin64(zb->z_lat);
    if ((abs(longitude) > 180) || (abs(latitude) > 90) || (abs(bin32(zb->z_alt)) > MAX_ALT)) {
        fprintf(stderr, "Bad GPS value!\n");
        free(zb);
        return;
    }

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

    longitude = bin64(n->zergblk->z_long);
    latitude = bin64(n->zergblk->z_lat);
    printf("Source: %d\tHP: %d/%u\tLong: %f deg\tLat: %f\tAlt: %6.4f\n",
           ntohs(n->zergblk->z_id),
           NTOH3(n->zergblk->z_hp),
           NTOH3(n->zergblk->z_maxhp),
           longitude,
           latitude,
           bin32(n->zergblk->z_alt));
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

/*
 * function: printhealth
 * display nodes with low health.
 *
 * root: root of BST to search through.
 * hp: percentage threshold below which nodes will be displayed.
 */
void printhealth(const Node *root, const double hp)
{
    if (root) {
        printhealth(root->left, hp);

        if (!root->zergblk)
            return;
        double z_per = (double) _ntoh3(root->zergblk->z_hp) / _ntoh3(root->zergblk->z_maxhp);
        z_per *= 100;
        if (z_per < hp)
            printf("Zerg: %d\tHP: %f\n", ntohs(root->zergblk->z_id), z_per);

        printhealth(root->right, hp);
    }
}

/*
 * function: rmtree
 * recursively deallocate memory used by a specifed BST.
 *
 * root: root of BST to deallocate
 */
void rmtree(Node *root)
{
    if (root) {
        rmtree(root->left);
        rmtree(root->right);
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
