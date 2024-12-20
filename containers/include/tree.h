#ifndef TREE_H
#define TREE_H

#include <stdint.h>

typedef int (*cmpFunction_t)(const void *a, const void *b);
typedef int (*printFunction_t)(void *buffer, const void *a);

enum treeStatus {
    TREE_SUCCESS = 0,
    TREE_ERROR   = 1,

    TREE_PARENT_ERROR,
    TREE_SORT_ERROR
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

enum treeStatus treeVerify(const node_t *tree, cmpFunction_t cmp);
enum treeStatus treeDump(const node_t *tree, printFunction_t print);

/// @brief Insert using cmp function
/// WARNING: Function doesn't do anything if cmp == NULL
enum treeStatus treeInsert(node_t *tree, const void *elem, cmpFunction_t cmp);

/// @brief Add leaf to the node
/// If leaf exists, overwrites it's value
/// @param right If true, add to the right subtree, else to the left subtree
/// @return Pointer to added node, NULL in case of error
node_t *treeAdd(node_t *node, const void *elem, bool right);

/// @brief Find given element in tree
/// If cmp == NULL uses memcmp
node_t *treeFind(node_t *tree, const void *elem, cmpFunction_t cmp);

node_t *treeSortFind(node_t *tree, const void *elem, cmpFunction_t cmp);

node_t *nodeCtor(const void *elem, node_t *parent, size_t elemSize);

#ifndef NDEBUG

#define TREE_ASSERT(node, cmp)                                                                  \
        do {                                                                                    \
            enum treeStatus status = treeVerify(node, cmp);                                     \
            if (status != TREE_SUCCESS) {                                                       \
                logPrint(L_ZERO, 1, "%s:%d | %s:Tree[%p] error occurred: error_code = %d\n",    \
                        __FILE__, __LINE__, __PRETTY_FUNCTION__, node, status);                 \
                return status;                                                                  \
            }                                                                                   \
        } while(0)

#define TREE_NODE_ASSERT(node, cmp)                                                             \
        do {                                                                                    \
            enum treeStatus status = treeVerify(node, cmp);                                     \
            if (status != TREE_SUCCESS) {                                                       \
                logPrint(L_ZERO, 1, "%s:%d | %s:Tree[%p] error occurred: error_code = %d\n",    \
                        __FILE__, __LINE__, __PRETTY_FUNCTION__, node, status);                 \
                return NULL;                                                                    \
            }                                                                                   \
        } while(0)

#else

#define TREE_ASSERT(...)
#define TREE_NODE_ASSERT(...)

#endif



#endif
