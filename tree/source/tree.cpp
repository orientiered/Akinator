#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <wchar.h>

#include "error_debug.h"
#include "logger.h"
#include "utils.h"
#include "cList.h"
#include "tree.h"

enum treeStatus treeCtor(node_t *tree, const void *elem, size_t elemSize) {
    MY_ASSERT(tree, abort());
    logPrint(L_EXTRA, 0, "Constructing tree[%p] with root data[%p]\n", tree, elem);

    tree->elemSize = elemSize;
    tree->parent = tree->left = tree->right = NULL;
    tree->data = calloc(1, elemSize);
    memcpy(tree->data, elem, elemSize);
    TREE_ASSERT(tree, NULL);
    return TREE_SUCCESS;
}

enum treeStatus treeDtor(node_t *tree) {
    MY_ASSERT(tree, abort());
    TREE_ASSERT(tree, NULL);
    logPrint(L_EXTRA, 0, "Destructing tree[%p]\n", tree);

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

enum treeStatus treeVerify(const node_t *tree, cmpFunction_t cmp) {
    MY_ASSERT(tree, abort());
    if (tree->left != NULL) {
        if (tree->left->parent != tree) {
            logPrint(L_ZERO, 1, "[Error] tree[%p] != left.parent[%p]\n", tree, tree->left->parent);
            return TREE_PARENT_ERROR;
        }

        if (cmp != NULL && cmp(tree->left->data, tree->data) > 0) {
            logPrint(L_ZERO, 1, "[Error] Left element in node[%p] must be in right subtree\n", tree);
            return TREE_SORT_ERROR;
        }

        enum treeStatus leftStatus = treeVerify(tree->left, cmp);
        if (leftStatus != TREE_SUCCESS)
            return leftStatus;
    }
    if (tree->right != NULL) {
        if (tree->right->parent != tree) {
            logPrint(L_ZERO, 1, "[Error] tree[%p] != right.parent[%p]\n", tree, tree->right->parent);
            return TREE_PARENT_ERROR;
        }

        if (cmp != NULL && cmp(tree->right->data, tree->data) <= 0) {
            logPrint(L_ZERO, 1, "[Error] Right element in node[%p] must be in left subtree\n", tree);
            return TREE_SORT_ERROR;
        }

        enum treeStatus rightStatus = treeVerify(tree->right, cmp);
        if (rightStatus != TREE_SUCCESS)
            return rightStatus;
    }
    return TREE_SUCCESS;
}

enum treeStatus treeDump(const node_t *tree, printFunction_t sPrint) {
    static size_t dumpCounter = 0;
    dumpCounter++;
    system("mkdir -p logs/dot logs/img");

    char buffer[128] = "";
    sprintf(buffer, "logs/dot/tree_%zu.dot", dumpCounter);
    FILE *dotFile = fopen(buffer, "wb");
    fwprintf(dotFile, L"digraph {\n"
                      L"graph [splines=line]\n");

    cList_t toVisit = {0};
    listCtor(&toVisit, sizeof(node_t *));
    listPushFront(&toVisit, &tree);

    wchar_t valueBuffer[128] = L"";
    while (toVisit.size > 0) {
        node_t *current = *(node_t **)listGet(&toVisit, listFront(&toVisit));
        listPopFront(&toVisit);

        if (current->right)
            listPushFront(&toVisit, &(current->right));
        if (current->left)
            listPushFront(&toVisit, &(current->left));

        sPrint(valueBuffer, current->data);
        int errCOde = fwprintf(dotFile, L"\tnode%p [shape = Mrecord, label = \"{node[%p] | parent[%p] | value = %ls | { <left>left[%p] | <right>right[%p] }}\"];\n",
                          current,                           current,  current->parent,  valueBuffer, current->left, current->right);
        if (current->left)
            fwprintf(dotFile, L"\tnode%p:<left> -> node%p;\n", current, current->left);
        if (current->right)
            fwprintf(dotFile, L"\tnode%p:<right> -> node%p;\n", current, current->right);
    }
    listDtor(&toVisit);
    fwprintf(dotFile, L"}\n");
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
    TREE_ASSERT(tree, cmp);

    node_t *parent = tree;
    node_t **leaf = cmp(elem, tree->data) <= 0 ? &tree->left : &tree->right;
    while (*leaf != NULL) {
        parent = *leaf;
        leaf = cmp(elem, parent->data) <= 0 ? &(parent->left) : &(parent->right);
    }

    node_t *newNode = nodeCtor(elem, parent, parent->elemSize);
    *leaf = newNode;

    TREE_ASSERT(tree, cmp);
    return TREE_SUCCESS;
}

node_t *treeAdd(node_t *node, const void *elem, bool right) {
    MY_ASSERT(node, abort());
    MY_ASSERT(elem, abort());
    TREE_NODE_ASSERT(node, NULL);

    node_t *newNode = nodeCtor(elem, node, node->elemSize);

    if (right)
        node->right = newNode;
    else
        node->left  = newNode;

    TREE_NODE_ASSERT(node, NULL);
    return newNode;
}

node_t *nodeCtor(const void *elem, node_t *parent, size_t elemSize) {
    MY_ASSERT(elem, abort());

    if (elemSize == 0) {
        logPrint(L_ZERO, 1, "[Warning] Try to create node with elemSize = 0");
    }
    node_t *newNode = (node_t *)calloc(1, sizeof(node_t));
    newNode->parent = parent;
    newNode->elemSize = elemSize;
    newNode->data = calloc(1, elemSize);

    if (!newNode->data) {
        logPrint(L_ZERO, 1, "[Error] Allocating of node->data failed: size = %zu\n", elemSize);
        free(newNode);
        return NULL;
    }

    memcpy(newNode->data, elem, elemSize);

    return newNode;
}

node_t *treeFind(node_t *tree, const void *elem, cmpFunction_t cmp) {
    MY_ASSERT(tree, abort());
    MY_ASSERT(elem, abort());
    TREE_NODE_ASSERT(tree, NULL);

    if (cmp == NULL && memcmp(tree->data, elem, tree->elemSize) == 0)
        return tree;
    else if (cmp(tree->data, elem) == 0) return tree;

    node_t *result = NULL;
    if (tree->left)
        result = treeFind(tree->left, elem, cmp);
    if (result) return result;

    if (tree->right)
        result = treeFind(tree->right, elem, cmp);

    return result;
}

node_t *treeSortFind(node_t *tree, const void *elem, cmpFunction_t cmp) {
    MY_ASSERT(tree, abort());
    MY_ASSERT(elem, abort());
    TREE_NODE_ASSERT(tree, cmp);

    if (cmp == NULL) {
        logPrint(L_ZERO, 1, "Tree sort find can't be used without cmp function\n");
        return NULL;
    }

    node_t *result = tree;
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
