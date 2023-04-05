/* Idea borrowed from Introduction to Algorithms, CLSR
 *
 * Database systems use B-tree to store information like singleton primary
 * key of data as KEY and data pointer in secondary storage as DATA. Each
 * BTREE node, except a few, also stored in secondary storage. We choose T
 * "MINIMUN DEGREE" such a way that the size of a BTREE node equals to
 *  page size of secondary storage.
 *
 * For this implementation, we store everything in HEAP instead of
 * secondary. There is no DATA in BTREE node in this implementation but
 * in actual implementation, we need to change KEY-DATA simultaneously.
 *
 * The algorithms are (mostly) ONE-PASS.
 **/

#include <stdio.h>
#include <stdlib.h>

#define t 2

typedef struct btree {
    int keys[2 * t - 1];
    // datatype data[2 * t - 1];
    int count;  // #keys currently stored
    int isLeaf; // 1 if leaf, 0 o/w
    struct btree * children[2 * t];
} btree;

btree *root = NULL;

/* Create an empty btree node
 * @param isLeaf Value 1 if leaf, 0 o/w
 * @return pointer of created node
 **/
btree* newNode(int isLeaf) {
    btree *temp = calloc(1, sizeof(btree));

    temp->count = 0;
    temp->isLeaf = isLeaf;

    return temp;
}

/* Split the full node NODE->CHILDREN[I] about its median key, which
 * moves up into NODE that separates new children
 *
 * @param node nonfull
 * @param i index of full child
 **/
void splitChild(btree *node, int i) {
    btree *left = node->children[i];
    btree *right = newNode(left->isLeaf);

    for (int j = 0; j < t - 1; j++)
        right->keys[j] = left->keys[j + t];

    if (left->isLeaf != 1) {
        for (int j = 0; j < t; j++)
            right->children[j] = left->children[j + t];
    }

    left->count = t - 1;
    right->count = t - 1;

    for (int j = node->count - 1; j >= i; j--)
        node->keys[j + 1] = node->keys[j];

    node->keys[i] = left->keys[t - 1];

    for (int j = node->count; j > i; j--)
        node->children[j + 1] = node->children[j];

    node->children[i + 1] = right;

    node->count = node->count + 1;
}

/* Insert KEY into subtree rooted at NODE
 * @invariant NODE is not full
 **/
void insertNonfull(btree *node, int key) {
    int i = node->count - 1;

    // insert KEY only into leaf
    if (node->isLeaf) {
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }

        node->keys[i + 1] = key;
        node->count = node->count + 1;

        return;
    }

    // find child to which the recursion decends
    while (i >= 0 && key < node->keys[i])
        i--;

    i = i + 1;

    if (node->children[i]->count == 2 * t - 1) {
        splitChild(node, i);

        if (key > node->keys[i])
            i++;
    }

    insertNonfull(node->children[i], key);
}

/* Insert KEY (not present already) into btree */
void insert(int key) {
    if (root == NULL)
        root = newNode(1);

    if (root->count == 2 * t - 1) {
        // Splitting the root is only way to increase the height
        btree *temp = newNode(0);

        temp->children[0] = root;
        root = temp;

        splitChild(root, 0);
    }

    insertNonfull(root, key);
}

/* Search for KEY in subtree rooted at NODE
 * @return 1 if present, 0 o/w
 **/
int search(btree *node, int key) {
    int i = 0;
    while (i < node->count && key > node->keys[i])
        i++;

    if (i < node->count && key == node->keys[i])
        return 1;
    else if (node->isLeaf)
        return 0;
    else
        return search(node->children[i], key);
}

/* Print keys of subtree rooted at NODE in order */
void traverse(btree *node) {
    if (node == NULL)
        return;

    int i = 0;

    if (node->isLeaf) {
        for (; i < node->count; i++)
            printf("%d ", node->keys[i]);
    } else {
        for (; i < node->count; i++) {
            traverse(node->children[i]);
            printf("%d ", node->keys[i]);
        }

        traverse(node->children[i]);
    }
}

/* Return predecessor of NODE->KEYS[I]
 * @param node nonleaf
 * @param i
 **/
int getPred(btree *node, int i) {
    btree *curr = node->children[i];
    while (curr->isLeaf != 1)
        curr = curr->children[curr->count];

    return curr->keys[curr->count - 1];
}

/* Return successor of NODE->KEYS[I]
 * @param node nonleaf
 * @param i
 **/
int getSucc(btree *node, int i) {
    btree *curr = node->children[i + 1];
    while (curr->isLeaf != 1)
        curr = curr->children[0];

    return curr->keys[0];
}

/* Right rotation around NODE->KEYS[I - 1]
 *
 * NODE->CHILDREN[I]->COUNT < T
 * NODE->CHILDREN[I - 1]->COUNT >= T
 **/
void rotateRight(btree *node, int i) {
    btree *left = node->children[i - 1];
    btree *right = node->children[i];

    for (int j = right->count - 1; j >= 0; j--)
        right->keys[j + 1] = right->keys[j];

    right->keys[0] = node->keys[i - 1];
    node->keys[i - 1] = left->keys[left->count - 1];

    if (left->isLeaf != 1) {
        for (int j = right->count - 1; j >= 0 ; j--)
            right->children[j + 1] = right->children[j];

        right->children[0] = left->children[left->count];
    }

    left->count = left->count - 1;
    right->count = right->count + 1;
}

/* Left rotation around NODE->KEYS[I]
 *
 * NODE->CHILDREN[I]->COUNT < T
 * NODE->CHILDREN[I + 1]->COUNT >= T
 **/
void rotateLeft(btree *node, int i) {
    btree *left = node->children[i];
    btree *right = node->children[i + 1];

    left->keys[left->count] = node->keys[i];
    node->keys[i] = right->keys[0];

    for (int j = 1; j < right->count; j++)
        right->keys[j - 1] = right->keys[j];

    if (left->isLeaf != 1) {
        left->children[left->count + 1] = right->children[0];

        for (int j = 0; j < right->count ; j++)
            right->children[j] = right->children[j + 1];
    }

    left->count = left->count + 1;
    right->count = right->count - 1;
}

/* Merge NODE->CHILDREN[I] and NODE->CHILDREN[I + 1]
 *
 * NODE->CHILDREN[I]->COUNT == T - 1
 * NODE->CHILDREN[I + 1]->COUNT == T - 1
 **/
void merge(btree *node, int i) {
    btree *left = node->children[i];
    btree *right = node->children[i + 1];

    left->keys[t - 1] = node->keys[i];

    for (int j = 0; j < t - 1; j++)
        left->keys[j + t] = right->keys[j];

    if (left->isLeaf != 1) {
        for (int j = 0; j < t ; j++)
            left->children[j + t] = right->children[j];
    }

    for (int j = i + 1; j < node->count; j++) {
        node->keys[j - 1] = node->keys[j];
        node->children[j] = node->children[j + 1];
    }

    left->count = 2 * t - 1;
    node->count = node->count - 1;

    free(right);
}

/* Delete I-th key of leaf NODE */
void deleteFromLeaf(btree *node, int i) {
    for (int j = i + 1; j < node->count; j++)
        node->keys[j - 1] = node->keys[j];

    node->count = node->count - 1;
}

void delete(btree *node, int key);

/* Delete I-th key of nonleaf NODE */
void deleteFromNonLeaf(btree *node, int i) {
    // TODO: find pred/succ and delete it in a single downward pass
    int key;

    if (node->children[i]->count >= t) {
        key = getPred(node, i);
        node->keys[i] = key;
        delete(node->children[i], key);
    } else if (node->children[i + 1]->count >= t) {
        key = getSucc(node, i);
        node->keys[i] = key;
        delete(node->children[i + 1], key);
    } else {
        key = node->keys[i];
        merge (node, i);
        delete(node->children[i], key);
    }
}

/* Delete KEY from subtree rooted at NODE
 *
 * @invariant #keys >= t in NODE (except root)
 **/
void delete(btree *node, int key) {
    int i = 0;
    while (i < node->count && key > node->keys[i])
        i++;

    if (i < node->count && key == node->keys[i]) {
        if (node->isLeaf)
            deleteFromLeaf(node, i);
        else
            deleteFromNonLeaf(node, i);
    } else if (node->isLeaf) {
        fprintf(stderr, "key doesn't exit!\n");
    } else {
        if (node->children[i]->count < t) {
            if (i != 0 && node->children[i - 1]->count >= t) {
                rotateRight(node, i);
            } else if (i != node->count && node->children[i + 1]->count >= t) {
                rotateLeft(node, i);
            } else {
                if (i == node->count)
                    i--;

                merge(node, i);
            }
        }

        delete(node->children[i], key);
    }

    // only possible when NODE == ROOT
    if (node->count == 0) {
        if (node->isLeaf) {
            free(node);
            root = NULL;
        } else {
            // only way to decrease the height of the tree by one
            btree *temp = node;
            root = node->children[0];
            free(temp);
        }
    }
}

int main() {
    int keys[] = {7, 11, 3, 10, 14, 13, 1, 15, 4, 5, 20, 22, 2, 17, 12, 6};
    int size = sizeof(keys) / sizeof(keys[0]);

    for (int i = 0; i < size; i++)
        insert(keys[i]);

    traverse(root);
    printf("\n\n");

    if (search(root, 21))
        printf("present\n");
    else
        printf("not present\n");
    printf("\n");

    int rm[] = {20, 22, 2, 1, 3, 7, 10, 21, 4, 5, 17, 12, 6};
    size = sizeof(rm) / sizeof(rm[0]);

    for (int i = 0; i < size; i++) {
        delete(root, rm[i]);
        traverse(root);
        printf(" [%d]\n", rm[i]);
    }
}
