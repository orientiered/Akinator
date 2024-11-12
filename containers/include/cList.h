#ifndef C_LIST_H
#define C_LIST_H

#define LIST_VERIFICATION 1

#include <stdint.h>

const int64_t LIST_POISON = 0x0FACEFABDDFAC;
const size_t MIN_LIST_RESERVED = 4;
const size_t SIZE_MULTIPLIER = 2;
enum listStatus {
    LIST_NULL_PTR_ERROR = -1,
    LIST_SUCCESS = 0,
    LIST_ERROR   = 1,
    LIST_MEMORY_ERROR,     ///< Calloc failed; Data, prev or next is NULL
    LIST_SIZE_ERROR,       ///< Size < 0 or reserved < 0 or size > reserved or elemSize <= 0

    LIST_NEXT_LINK_ERROR,  ///< Wrong linking in next array
    LIST_FREE_LINK_ERROR,  ///< Wrong linking in free array

    LIST_PREV_LINK_ERROR,  ///< Wrong linking in prev array

    LIST_HEAD_ERROR,       ///< Prev[head] != 0         or 0 < iter || iter > reserved
    LIST_TALE_ERROR,       ///< Next[tale] != 0
    LIST_FREE_ERROR,       ///< Prev[free] != -1
};

typedef int32_t listIterator_t;
typedef int (*listPrintFunction_t)(char *buffer, const void *a);

const listIterator_t INVALID_LIST_IT = -1;
const listIterator_t NULL_LIST_IT = 0;
typedef struct cList {
    int32_t  size;
    int32_t  reserved;

    size_t   elemSize;
    void    *data;

    int32_t *next;
    int32_t *prev;

    int32_t free;
    listPrintFunction_t sPrint;
} cList_t;

/// @brief Construct list with elements of elemSize
enum listStatus listCtor(cList_t *list, size_t elemSize, listPrintFunction_t sPrint);

/// @brief Descturct list
/// WARNING: listDtor shouldn't be called on destructed or not initialized list
enum listStatus listDtor(cList_t *list);

/// @brief Remove all elements from list
enum listStatus listClear(cList_t *list);

/*! @brief Check list on logic errors
    Prints some info about error
    @return Corresponding error code
!*/
enum listStatus listVerify(cList_t *list);

/// @brief Graphical list dump
/// NOTE: log must be opened in L_HTML_MODE
enum listStatus listDump(cList_t *list, const char *callMessage);

#define LIST_DUMP(list, msg)                                                        \
        do {                                                                        \
            logPrintWithTime(L_ZERO, 0, "<b>cList_t dump</b>\n"                     \
                                "Called from %s:%d\n", __FILE__, __LINE__);         \
            listDump(list, msg);                                                    \
        } while(0)

/// @brief Return head of list
listIterator_t  listFront(cList_t *list);

/// @brief Return tale of list
listIterator_t  listBack(cList_t *list);

/// @brief Return next element
listIterator_t  listNext(cList_t *list, listIterator_t iter);

/// @brief Return previous element
listIterator_t  listPrev(cList_t *list, listIterator_t iter);

/// @brief Push elem before head
listIterator_t  listPushFront(cList_t *list, const void *elem);

/// @brief Push elem after tale
listIterator_t  listPushBack(cList_t *list, const void *elem);

/// @brief Pop head element
listIterator_t listPopFront(cList_t *list);

/// @brief Pop tale element
listIterator_t listPopBack(cList_t *list);

/// @brief Remove element by given iterator
enum listStatus listRemove(cList_t *list, listIterator_t iter);

/// @brief Find first occurrence of elem in list
/// @return Iterator to found elem, INVALID_LIST_IT otherwise
listIterator_t listFind(cList_t *list, const void *elem);

/// @brief Insert after iterator, return iterator to inserted elem
listIterator_t listInsertAfter(cList_t *list, listIterator_t iter, const void *elem);

/// @brief Insert before iterator, return iterator to inserted elem
listIterator_t listInsertBefore(cList_t *list, listIterator_t iter, const void *elem);

/// @brief Get value from given list node
/// @return Pointer to value, NULL otherwise
void *listGet(cList_t *list, listIterator_t iter);

#if defined(LIST_VERIFICATION) && !defined(NDEBUG)

# define LIST_ASSERT(list)                                                                          \
    do {                                                                                            \
        enum listStatus status = listVerify(list);                                                  \
        if (status != LIST_SUCCESS) {                                                               \
            logPrint(L_ZERO, 1, "<h2>%s:%d, %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__);        \
            logPrint(L_ZERO, 1, "List[%p] error occurred. Error code = %d</h2>\n", list, status);   \
            LIST_DUMP(list, "assert failed");                                                       \
            return status;                                                                          \
        }                                                                                           \
    } while(0)

/// @brief List assert used in functions returning custom error values
# define LIST_CUSTOM_ASSERT(list, ERR_VALUE)                                                        \
    do {                                                                                            \
        enum listStatus status = listVerify(list);                                                  \
        if (status != LIST_SUCCESS) {                                                               \
            logPrint(L_ZERO, 1, "<h2>%s:%d, %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__);        \
            logPrint(L_ZERO, 1, "List[%p] error occurred. Error code = %d</h2>\n", list, status);   \
            LIST_DUMP(list, "assert failed");                                                       \
            return ERR_VALUE;                                                                       \
        }                                                                                           \
    } while(0)

#else
# define LIST_ASSERT(list)
# define LIST_CUSTOM_ASSERT(list, ERR_VALUE)
#endif

#endif
