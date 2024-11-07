#ifndef TREE_H
#define TREE_H

#include <stdint.h>

typedef int (*cmpFunction_t)(const void *a, const void *b);
typedef int (*printFunction_t)(char *buffer, const void *a);

enum treeStatus {
    TREE_SUCCESS = 0,
    TREE_ERROR   = 1,

};


typedef struct node {
    // size_t size;        ///< Number of elements in tree starting from this node

    void *data;
    uint16_t elemSize;

    node *parent;
    node *left;
    node *right;
} node_t;

// typedef struct {
//     // size_t size;
//     node_t *root;
//
//     cmpFunction_t cmp;            ///< Comparator function
//     printFunction_t print;        ///< Print function for elements
// } Tree_t;

enum treeStatus treeCtor(node_t *tree, const void *elem, size_t elemSize);
enum treeStatus treeDtor(node_t *tree);

enum treeStatus treeVerify(node_t *tree);
enum treeStatus treeDump(node_t *tree, printFunction_t print);

/// @brief Insert using cmp function
/// WARNING: Function doesn't do anything if cmp == NULL
enum treeStatus treeInsert(node_t *tree, const void *elem, cmpFunction_t cmp);

/// @brief Add leaf to the node
/// If leaf exists, overwrites it's value
/// @param right If true, add to the right subtree, else to the left subtree
/// @return Pointer to added node, NULL in case of error
node_t *treeAdd(node_t *node, const void *elem, bool right);

/// @brief Find given element in tree
/// If cmp == NULL searches through all elements, else tree is considered as sorted
node_t *treeFind(node_t *tree, const void *elem, cmpFunction_t cmp);

node_t *nodeCtor(const void *elem, node_t *parent, size_t elemSize);



#endif
