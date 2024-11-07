#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "error_debug.h"
#include "logger.h"
#include "utils.h"
#include "cList.h"
#include "tree.h"

enum treeStatus treeCtor(node_t *tree, const void *elem, size_t elemSize) {
    MY_ASSERT(tree, abort());
    tree->elemSize = elemSize;
    tree->parent = tree->left = tree->right = NULL;
    tree->data = calloc(1, elemSize);
    memcpy(tree->data, elem, elemSize);
    return TREE_SUCCESS;
}

enum treeStatus treeDtor(node_t *tree) {
    MY_ASSERT(tree, abort());
    if (tree->parent != NULL)
        logPrint(L_DEBUG, 0, "[Warning] Destructing tree[%p] not from root\n", tree);
    if (tree->left != NULL) {
        tree->left->parent = (node_t*)NULL;
        treeDtor(tree->left);
    }
    if (tree->right != NULL) {
        tree->right->parent = (node_t*)NULL;
        treeDtor(tree->right);
    }
    free(tree->data);
    free(tree); tree = NULL;

    return TREE_SUCCESS;
}

enum treeStatus treeVerify(node_t *tree) {
    return TREE_SUCCESS;
}

enum treeStatus treeDump(node_t *tree, printFunction_t print) {
    static size_t dumpCounter = 0;
    dumpCounter++;
    system("mkdir -p logs/dot logs/img");

    char buffer[100] = "";
    sprintf(buffer, "logs/dot/tree_%zu.dot", dumpCounter);
    FILE *dotFile = fopen(buffer, "w");
    fprintf(dotFile, "digraph {\n");

    cList_t toVisit = {0};
    listCtor(&toVisit, sizeof(node_t *));
    listPushFront(&toVisit, &tree);
    while (toVisit.size > 0) {
        node_t *current = *(node_t **)listGet(&toVisit, listFront(&toVisit));
        listPopFront(&toVisit);

        if (current->right)
            listPushFront(&toVisit, &(current->right));
        if (current->left)
            listPushFront(&toVisit, &(current->left));

        print(buffer, current->data);
        fprintf(dotFile, "\tnode%p [shape = Mrecord, label = \"{node[%p] | parent[%p] | value = %s | { left[%p] | right[%p] }}\"];\n",
                          current,                           current,  current->parent,  buffer, current->left, current->right);

        if (current->left)
            fprintf(dotFile, "\tnode%p -> node%p\n", current, current->left);
        if (current->right)
            fprintf(dotFile, "\tnode%p -> node%p\n", current, current->right);
    }
    listDtor(&toVisit);
    fprintf(dotFile, "}\n");
    fclose(dotFile);

    sprintf(buffer, "dot logs/dot/tree_%zu.dot -Tsvg -o logs/img/tree_dump_%zu.svg", dumpCounter, dumpCounter);
    system(buffer);

    logPrint(L_ZERO, 0, "<img src=\"img/tree_dump_%zu.svg\" width=76%%>\n<hr>\n", dumpCounter);
    return TREE_SUCCESS;
}

enum treeStatus treeInsert(node_t *tree, const void *elem, cmpFunction_t cmp) {
    MY_ASSERT(tree, abort());
    MY_ASSERT(elem, abort());
    MY_ASSERT(cmp, abort());

    node_t *parent = tree;
    node_t *leaf = cmp(elem, tree->data) <= 0 ? tree->left : tree->right;
    while (leaf != NULL) {
        parent = leaf;
        leaf = cmp(elem, leaf->data) <= 0 ? leaf->left : leaf->right;
    }

    node_t *newNode = nodeCtor(elem, parent, parent->elemSize);
    leaf = newNode;

    return TREE_SUCCESS;
}

node_t *treeAdd(node_t *node, const void *elem, bool right) {
    MY_ASSERT(node, abort());
    MY_ASSERT(elem, abort());

    node_t *newNode = nodeCtor(elem, node, node->elemSize);

    if (right)
        node->right = newNode;
    else
        node->left  = newNode;

    return newNode;
}

node_t *nodeCtor(const void *elem, node_t *parent, size_t elemSize) {
    node_t *newNode = (node_t *)calloc(1, sizeof(node_t));
    newNode->parent = parent;
    newNode->elemSize = elemSize;
    newNode->data = calloc(1, elemSize);
    memcpy(newNode->data, elem, elemSize);

    return newNode;
}

node_t *treeFind(node_t *tree, const void *elem, cmpFunction_t cmp) {
    MY_ASSERT(tree, abort());
    MY_ASSERT(elem, abort());

    node_t *result = NULL;
    if (cmp == NULL) {
        // search through all elements
        if (cmp(tree->data, elem) == 0) return tree;

        if (tree->left)
            result = treeFind(tree->left, elem, cmp);
        if (result) return result;

        if (tree->right)
            result = treeFind(tree->right, elem, cmp);

        return result;
    }

    result = tree;
    int cmpResult = 0;
    do {
        cmpResult = cmp(elem, result->data);
        if (cmpResult < 0)
            result = result->left;
        else
            result = result->right;
    } while (cmpResult != 0 && result != NULL);
    return result;
}

