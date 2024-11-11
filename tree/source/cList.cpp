#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "error_debug.h"
#include "logger.h"
#include "cList.h"

const size_t INTERNAL_BUFFER_SIZE = 100;

static bool checkIfInvalidIterator(cList_t *list, listIterator_t iter) {
    MY_ASSERT(list, abort());
    return (iter < 0 || iter > list->reserved);
}

static enum listStatus listRealloc(cList_t *list) {
    MY_ASSERT(list, abort());
    LIST_ASSERT(list);
    logPrint(L_DEBUG, 0, "Reallocating list [%p]: %d -> %d\n", list, list->reserved, list->reserved * 2);

    // + 1 because NULL list element isn't counted
    size_t newCapacity = (list->reserved * 2 + 1);

    int32_t *newNext = (int32_t *) realloc(list->next, sizeof(int32_t) * newCapacity);
    if (!newNext) {
        logPrint(L_ZERO, 1, "Reallocation of cList_t[%p]::next[%p] failed\n", list, list->next);
        return LIST_MEMORY_ERROR;
    }
    list->next = newNext;

    int32_t *newPrev = (int32_t *) realloc(list->prev, sizeof(int32_t) * newCapacity);
    if (!newPrev) {
        logPrint(L_ZERO, 1, "Reallocation of cList_t[%p]::prev[%p] failed\n", list, list->prev);
        return LIST_MEMORY_ERROR;
    }
    list->prev = newPrev;

    void *newData = realloc(list->data, list->elemSize * newCapacity);
    if (!newData) {
        logPrint(L_ZERO, 1, "Reallocation of cList_t[%p]::data[%p] failed\n", list, list->data);
        return LIST_MEMORY_ERROR;
    }
    list->data = newData;

    for (int32_t idx = list->reserved + 1; idx <= list->reserved * 2; idx++) {
        list->prev[idx] = INVALID_LIST_IT;
        list->next[idx] = idx + 1; //free elements
        memcpy((char *)list->data + list->elemSize*idx, &LIST_POISON, list->elemSize);
    }

    list->next[list->reserved * 2] = NULL_LIST_IT;

    if (list->free == NULL_LIST_IT)
        list->free = list->reserved + 1;
    else {
        listIterator_t it = list->free;
        while (list->next[it] != NULL_LIST_IT)
            it = list->next[it];
        list->next[it] = list->reserved + 1;
    }
    list->reserved *= 2;

    LIST_ASSERT(list);
    return LIST_SUCCESS;
}

enum listStatus listCtor(cList_t *list, size_t elemSize) {
    MY_ASSERT(list, abort());
    logPrint(L_DEBUG, 0, "Constructing list [%p]\n", list);

    list->size     = 0;
    list->reserved = MIN_LIST_RESERVED;

    list->elemSize = elemSize;
    list->data     = calloc(MIN_LIST_RESERVED + 1, elemSize);

    list->next     = (int32_t *) calloc(MIN_LIST_RESERVED + 1, sizeof(int32_t));
    list->prev     = (int32_t *) calloc(MIN_LIST_RESERVED + 1, sizeof(int32_t));

    //first element is reserved, it points to itself

    //next stores two sequences:
    //1. indexes of data elements
    //2. indexes of free elements
    for (int32_t idx = 1; idx <= list->reserved; idx++) {
        list->next[idx] = idx + 1; //filling free sequence
        list->prev[idx] = -1;
        memcpy((char *)list->data + elemSize * idx, &LIST_POISON, elemSize);
    }
    memcpy(list->data, &LIST_POISON, elemSize);
    list->next[list->reserved] = 0; // next(last) = 0

    list->next[0] = 0;
    list->prev[0] = 0;
    list->free    = 1;

    LIST_ASSERT(list);

    logPrint(L_DEBUG, 0, "Constructed list [%p] successfully\n", list);
    return LIST_SUCCESS;
}

enum listStatus listDtor(cList_t *list) {
    MY_ASSERT(list, abort());
    LIST_ASSERT(list);
    logPrint(L_DEBUG, 0, "Destructing list [%p]\n", list);
    free(list->data); list->data = NULL;
    free(list->next); list->next = NULL;
    free(list->prev); list->prev = NULL;

    logPrint(L_DEBUG, 0, "Destructed list [%p] successfully\n", list);
    return LIST_SUCCESS;
}

enum listStatus listClear(cList_t *list) {
    MY_ASSERT(list, abort());
    LIST_ASSERT(list);
    logPrint(L_DEBUG, 0, "Clearing list [%p]\n", list);

    list->size = 0;
    list->next[0] = 0;
    list->prev[0] = 0;
    list->free = 1;
    for (int32_t idx = 1; idx <= list->reserved; idx++) {
        list->next[idx] = idx + 1; //filling free sequence
        list->prev[idx] = -1;
        memcpy((char *)list->data + list->elemSize * idx, &LIST_POISON, list->elemSize);
    }
    memcpy(list->data, &LIST_POISON, list->elemSize);
    list->next[list->reserved] = 0;

    LIST_ASSERT(list);
    logPrint(L_DEBUG, 0, "Cleared list [%p]\n", list);
    return LIST_SUCCESS;
}

listIterator_t  listFront(cList_t *list) {
    MY_ASSERT(list, abort());
    LIST_CUSTOM_ASSERT(list, INVALID_LIST_IT);

    return (listIterator_t) list->next[0];
}

listIterator_t  listBack(cList_t *list) {
    MY_ASSERT(list, abort());
    LIST_CUSTOM_ASSERT(list, INVALID_LIST_IT);

    return (listIterator_t) list->prev[0];
}

listIterator_t  listNext(cList_t *list, listIterator_t iter) {
    MY_ASSERT(list, abort());
    MY_ASSERT(iter, abort());
    LIST_CUSTOM_ASSERT(list, INVALID_LIST_IT);

    if (checkIfInvalidIterator(list, iter)) {
        logPrint(L_DEBUG, 0, "Invalid listIterator_t passed in listNext: %ld\n"
                             "For list[%p] maximum iterator is %ld\n",
                             iter, list, list->reserved);
        return (listIterator_t) INVALID_LIST_IT;
    }

    return list->next[iter];
}

listIterator_t  listPrev(cList_t *list, listIterator_t iter) {
    MY_ASSERT(list, abort());
    MY_ASSERT(iter, abort());
    LIST_CUSTOM_ASSERT(list, INVALID_LIST_IT);

    if (checkIfInvalidIterator(list, iter)) {
        logPrint(L_DEBUG, 0, "Invalid listIterator_t passed in listPrev: %ld\n"
                             "For list[%p] maximum iterator is %ld\n",
                             iter, list, list->reserved);
        return (listIterator_t) INVALID_LIST_IT;
    }

    return list->prev[iter];
}

listIterator_t  listPushFront(cList_t *list, const void *elem) {
    MY_ASSERT(list, abort());
    MY_ASSERT(elem, abort());
    LIST_CUSTOM_ASSERT(list, INVALID_LIST_IT);

    logPrint(L_EXTRA, 0, "Pushing element[%p] to front of list[%p]\n", elem, list);
    listInsertAfter(list, 0, elem);

    LIST_CUSTOM_ASSERT(list, INVALID_LIST_IT);
    return list->next[0];
}

listIterator_t  listPushBack(cList_t *list, const void *elem) {
    MY_ASSERT(list, abort());
    MY_ASSERT(elem, abort());
    LIST_CUSTOM_ASSERT(list, INVALID_LIST_IT);

    logPrint(L_EXTRA, 0, "Pushing element[%p] to back of list[%p]\n", elem, list);
    listInsertAfter(list, list->prev[0], elem);

    LIST_CUSTOM_ASSERT(list, INVALID_LIST_IT);
    return list->prev[0];
}

listIterator_t listPopFront(cList_t *list) {
    MY_ASSERT(list, abort());
    LIST_CUSTOM_ASSERT(list, INVALID_LIST_IT);

    logPrint(L_EXTRA, 0, "Popping element at front of list[%p]\n", list);
    if (listRemove(list, list->next[0]) != LIST_SUCCESS) {
        return INVALID_LIST_IT;
    }

    LIST_CUSTOM_ASSERT(list, INVALID_LIST_IT);
    return list->next[0];
}

listIterator_t listPopBack(cList_t *list) {
    MY_ASSERT(list, abort());
    LIST_CUSTOM_ASSERT(list, INVALID_LIST_IT);

    logPrint(L_EXTRA, 0, "Popping element at back of list[%p]\n", list);
    if (listRemove(list, list->prev[0]) != LIST_SUCCESS) {
        return INVALID_LIST_IT;
    }

    LIST_CUSTOM_ASSERT(list, INVALID_LIST_IT);
    return list->prev[0];
}

enum listStatus listRemove(cList_t *list, listIterator_t iter) {
    MY_ASSERT(list, abort());
    LIST_ASSERT(list);

    if (checkIfInvalidIterator(list, iter)) {
        logPrint(L_DEBUG, 0, "Invalid listIterator_t passed in listRemove: %ld\n"
                             "For list[%p] maximum iterator is %ld\n",
                             iter, list, list->reserved);
        return LIST_ERROR;
    }

    logPrint(L_EXTRA, 0, "Popping element[%ld] of list[%p]\n", iter, list);

    if (list->size == 0) {
        logPrint(L_ZERO, 1, "Attempt to pop element from empty list[%p]\n", list);
        return LIST_ERROR;
    }

    memcpy((char *)list->data + iter * list->elemSize, &LIST_POISON, list->elemSize);
    int32_t nextElem = list->next[iter],
            prevElem = list->prev[iter];

    list->next[prevElem] = nextElem;
    list->prev[nextElem] = prevElem;

    list->prev[iter] = INVALID_LIST_IT;
    list->next[iter] = list->free;
    list->free = iter;

    list->size--;

    LIST_ASSERT(list);
    return LIST_SUCCESS;
}


listIterator_t listFind(cList_t *list, const void *elem) {
    MY_ASSERT(list, abort());
    MY_ASSERT(elem, abort());
    LIST_CUSTOM_ASSERT(list, INVALID_LIST_IT);

    listIterator_t iter = list->next[0];
    while (iter != NULL_LIST_IT && memcmp(listGet(list, iter), elem, list->elemSize) != 0)
        iter = listNext(list, iter);

    return (iter == NULL_LIST_IT) ? INVALID_LIST_IT : iter;
}

/// @brief insert After iterator, return iterator to inserted elem
listIterator_t listInsertAfter(cList_t *list, listIterator_t iter, const void *elem) {
    MY_ASSERT(list, abort());
    MY_ASSERT(elem, abort());
    LIST_CUSTOM_ASSERT(list, INVALID_LIST_IT);

    logPrint(L_EXTRA, 0, "Inserting elem[%p] in list[%p] after [%d] iterator\n", elem, list, iter);

    if (checkIfInvalidIterator(list, iter)) {
        logPrint(L_DEBUG, 0, "Invalid listIterator_t passed in listInsertAfter: %ld\n"
                             "For list[%p] maximum iterator is %ld\n",
                             iter, list, list->reserved);
        return INVALID_LIST_IT;
    }

    if (list->free == NULL_LIST_IT)
        listRealloc(list);

    int32_t newElem = list->free;
    list->free = list->next[list->free];

    list->prev[newElem] = iter;
    list->next[newElem] = list->next[iter];
    list->prev[list->next[iter]] = newElem;
    list->next[iter] = newElem;

    list->size++;

    memcpy(listGet(list, newElem), elem, list->elemSize);

    LIST_CUSTOM_ASSERT(list, INVALID_LIST_IT);
    return list->next[iter];
}

/// @brief insert Before iterator, return iterator to inserted elem
listIterator_t listInsertBefore(cList_t *list, listIterator_t iter, const void *elem) {
    MY_ASSERT(list, abort());
    MY_ASSERT(elem, abort());
    LIST_CUSTOM_ASSERT(list, INVALID_LIST_IT);

    return listInsertAfter(list, list->prev[iter], elem);
}

void *listGet(cList_t *list, listIterator_t iter) {
    MY_ASSERT(list, abort());
    // MY_ASSERT(iter, abort());
    LIST_CUSTOM_ASSERT(list, NULL);

    if (checkIfInvalidIterator(list, iter)) {
        logPrint(L_DEBUG, 0, "Invalid listIterator_t passed in listGet: %ld\n"
                             "For list[%p] maximum iterator is %ld\n",
                             iter, list, list->reserved);
        return NULL;
    }

    if (list->prev[iter] == INVALID_LIST_IT) {
        // if previous element is not defined, iter's empty element
        return NULL;
    }

    return (void *) ( (char *) list->data + list->elemSize * iter );
}

enum listStatus listVerify(cList_t *list) {
    MY_ASSERT(list, abort());
    /* CHECKING BASIC LOGIC*/
    if (list->reserved < 0) {
        logPrint(L_ZERO, 1, "Negative reserved elements in list[%p]: %ld\n", list, list->reserved);
        return LIST_SIZE_ERROR;
    }
    if (list->size > list->reserved) {
        logPrint(L_ZERO, 1, "Size greater than reserved in list[%p]: %ld > %ld\n", list, list->size, list->reserved);
        return LIST_SIZE_ERROR;
    }
    if (list->size < 0) {
        logPrint(L_ZERO, 1, "Negative size in list[%p]: %ld\n", list, list->size);
        return LIST_SIZE_ERROR;
    }
    if (list->elemSize <= 0) {
        logPrint(L_ZERO, 1, "Element size isn't positive in list[%p]: %ld\n", list, list->elemSize);
        return LIST_SIZE_ERROR;
    }

    if (!list->data && list->reserved >= 0) {
        logPrint(L_ZERO, 1, "Data isn't allocated in list [%p]\n", list);
        return LIST_MEMORY_ERROR;
    }
    if (!list->next && list->reserved >= 0) {
        logPrint(L_ZERO, 1, "Next isn't allocated in list [%p]\n", list);
        return LIST_MEMORY_ERROR;
    }
    if (!list->prev && list->reserved >= 0) {
        logPrint(L_ZERO, 1, "Prev isn't allocated in list [%p]\n", list);
        return LIST_MEMORY_ERROR;
    }

    /*CHECKING OBVIOUS HEAD, TALE AND FREE ERRORS*/
    int32_t head = list->next[0],
            tale = list->prev[0];
    if (checkIfInvalidIterator(list, head)){
        logPrint(L_ZERO, 1, "Head iterator in list [%p] is invalid: %ld\n", list, head);
        return LIST_HEAD_ERROR;
    }
    if (list->prev[head] != 0) {
        logPrint(L_ZERO, 1, "Head iterator in list [%p] is misplaced: prev[head] = %ld != 0\n",
                 list, list->prev[head]);
        return LIST_HEAD_ERROR;
    }

    if (checkIfInvalidIterator(list, tale)) {
        logPrint(L_ZERO, 1, "Tale iterator in list [%p] is invalid: %ld\n", list, tale);
        return LIST_TALE_ERROR;
    }
    if (list->next[tale] != 0) {
        logPrint(L_ZERO, 1, "Tale iterator in list [%p] is misplaced: next[tale] = %ld != 0\n",
                 list, list->next[tale]);
        return LIST_TALE_ERROR;
    }

    if (checkIfInvalidIterator(list, list->free)) {
        logPrint(L_ZERO, 1, "Free iterator in list [%p] is invalid: %ld\n", list, list->free);
        return LIST_FREE_ERROR;
    }
    if (list->free != 0 && list->prev[list->free] != -1) {
        logPrint(L_ZERO, 1, "Free iterator in list [%p] is misplaced: prev[free] = %ld != -1\n",
                 list, list->prev[list->free]);
        return LIST_FREE_ERROR;
    }

    /*CHECKING NEXT, FREE AND PREV ON LINKING ERRORS (FULL ELEMENT COVERAGE AND ABSENCE OF CYCLES)*/
    int32_t visitedCounter = 0;
    listIterator_t iter = head;
    //list->reserved + 5 to be sure that verifier has found cycle
    for (;iter != NULL_LIST_IT && visitedCounter < list->reserved + 5;iter = list->next[iter]) {
        if (list->prev[list->next[iter]] != iter) {
            logPrint(L_ZERO, 1, "prev[next[iter]] != iter in list [%p]:\n"
                                "\titer = %d\n"
                                "\tnext[iter] = %d\n"
                                "\tprev[next[iter]] = %d != %d\n",
                                list, iter, list->next[iter], list->prev[list->next[iter]], iter);
            return LIST_NEXT_LINK_ERROR;
        }
        visitedCounter++;
    }
    if (visitedCounter > list->reserved) {
        logPrint(L_ZERO, 1, "Found cycle in list [%p] in next array\n", list);
        return LIST_NEXT_LINK_ERROR;
    } else if (visitedCounter != list->size) {
        logPrint(L_ZERO, 1, "Wrong linking in list [%p] in next array\n", list);
        return LIST_NEXT_LINK_ERROR;
    }

    /*checking free*/
    iter = list->free;
    for (;iter != NULL_LIST_IT && visitedCounter < list->reserved + 5;iter = list->next[iter]) {
        //logPrint(L_EXTRA, 0, "it = %d\n", iter);
        if (list->prev[iter] != INVALID_LIST_IT) {
            logPrint(L_ZERO, 1, "Found previous element for element from free sequence in list [%p]\n", list);
            return LIST_FREE_LINK_ERROR;
        }
        visitedCounter++;
    }

    if (visitedCounter > list->reserved) {
        logPrint(L_ZERO, 1, "Found cycle in list [%p] in free(next) array\n", list);
        return LIST_FREE_LINK_ERROR;
    } else if (visitedCounter != list->reserved) {
        logPrint(L_ZERO, 1, "Wrong linking in list [%p] in free(next) array\n", list);
        return LIST_FREE_LINK_ERROR;
    }

    /*checking prev*/
    iter = tale;
    visitedCounter = 0;
    for (; iter != NULL_LIST_IT && visitedCounter < list->reserved + 5; iter = list->prev[iter]) {
        visitedCounter++;
    }
    if (visitedCounter > list->reserved) {
        logPrint(L_ZERO, 1, "Found cycle in list [%p] in prev array\n", list);
        return LIST_PREV_LINK_ERROR;
    } else if (visitedCounter != list->size) {
        logPrint(L_ZERO, 1, "Wrong linking in list [%p] in prev array\n", list);
        return LIST_FREE_LINK_ERROR;
    }

    return LIST_SUCCESS;
}

enum listStatus listDump(cList_t *list) {
    static size_t imgNumber = 0;
    char buffer[INTERNAL_BUFFER_SIZE] = "";

    MY_ASSERT(list, abort());
    if (getLogLevel() < L_DEBUG)
        return LIST_SUCCESS;

    logPrintColor(L_ZERO, "#FF0000", "#CCCCCC", "-------cList_t [%p] dump--------\n", list);

    system("mkdir -p logs/img logs/dot");

    sprintf(buffer, "logs/dot/listDump_%zu.dot", imgNumber);
    FILE *dotFile = fopen(buffer, "w");
    fprintf(dotFile, "digraph {\n");
    fprintf(dotFile, "\trankdir = LR;\n");
    fprintf(dotFile, "\tgraph [splines=ortho];\n");

    fprintf(dotFile, "\tnodeHeader [shape=Mrecord, weight=10, label=\"Info | size = %d | capacity = %d\"]\n",
            list->size, list->reserved);

    fprintf(dotFile, "\tnode0 [shape=Mrecord, weight=10, label=\"NULL_ELEMENT | (head) next = %d | (tale) prev = %d\"",
            list->next[0], list->prev[0]);
    fprintf(dotFile, "\tcolor=\"#000000\"];\n");

    fprintf(dotFile, "\tsubgraph cluster_Data {\n");
    fprintf(dotFile, "\t\tlabel = \"Elements\";\n");
    fprintf(dotFile, "\t\tbgcolor=\"#ccfdf9\";\n");
    for (int32_t idx = 1; idx <= list->reserved; idx++) {
        fprintf(dotFile, "\t\tnode%d [shape=Mrecord, style=filled,weight=10, label=\"elem #%d | next = %d | prev = %d | val = %d\",",
                idx, idx, list->next[idx], list->prev[idx], *(int32_t *)((char *)list->data + idx * list->elemSize) );

        if (list->prev[idx] == -1)
            fprintf(dotFile, "fillcolor=\"#AAAAAA\"];\n");
        else
            fprintf(dotFile, "fillcolor=\"white\"];\n");
    }
    fprintf(dotFile, "\t}\n");

    for (int32_t idx = 0; idx <= list->reserved; idx++) {
        if (idx != list->reserved)
            fprintf(dotFile, "\tnode%d -> node%d [color=\"#00000000\"];\n", idx, idx+1);

        if (list->prev[idx] == -1)
            fprintf(dotFile, "\tnode%d -> node%d [constraint=false,color=\"#AAAAAA\"];\n", idx, list->next[idx]);
        else {
            fprintf(dotFile, "\tnode%d -> node%d [constraint=false,color=\"#20EE20\"];\n", idx, list->next[idx]);
            fprintf(dotFile, "\tnode%d -> node%d [constraint=false,color=\"#FF4420\"];\n", idx, list->prev[idx]);
        }

    }

    fprintf(dotFile, "\tnodeFree [shape = Mrecord, style = filled, weight = 20, label = \"Free | next = %d\"];\n"
                     "\tnodeFree -> node%d [color = \"#111111\"];\n", list->free, list->free);
    fprintf(dotFile, "}\n");
    fclose(dotFile);

    sprintf(buffer, "dot logs/dot/listDump_%zu.dot -Tsvg -o logs/img/dumpImg_%zu.svg",
            imgNumber, imgNumber);
    system(buffer);

    logPrint(L_ZERO, 0, "<img src=\"img/dumpImg_%zu.svg\" width=87%%>", imgNumber);
    logPrint(L_ZERO, 0, "\n<hr>\n");
    logFlush();

    imgNumber++;
    return LIST_SUCCESS;
}
