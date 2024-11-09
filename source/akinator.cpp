#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "logger.h"

#include "tree.h"
#include "akinator.h"

static node_t *NODE_null = (node_t *) size_t(-1);

static int sPrint(void *buffer, const void *a) {
    return swprintf((wchar_t*)buffer, MAX_LABEL_LEN, (const wchar_t*) a);
}

static node_t *recursiveReadDataBase(FILE *dataBase, node_t *parent) {
    wchar_t buffer[AKINATOR_BUFFER_SIZE] = L"";
    size_t scannedCounter = 0;

    // wlogPrint(L_DEBUG, 0, L"Scanning token:\n");
    fwscanf(dataBase, L" %*1[\"]%l[^\"]%*1[\"]", buffer);
    wlogPrint(L_DEBUG, 0, L"Akinator: read token '%ls'\n", buffer);

    bool nullNode = wcsncasecmp(L"_null", buffer, 5) == 0;
    logFlush();


    node_t *newNode = NULL;
    if (nullNode)
        logPrint(L_DEBUG, 0, "\t'_null' node, don't creating it\n");
    else
        newNode = nodeCtor(buffer, parent, sizeof(wchar_t) * MAX_LABEL_LEN);


    fwscanf(dataBase, L" : %n", &scannedCounter);
    if (scannedCounter != 0) {
        if (nullNode) {
            logPrint(L_ZERO, 1, "[Wrong format]Expected ; after '_null' node, got :\n");
            return NULL;
        }

        wlogPrint(L_ZERO, 0, L"\tScanning children\n");
        newNode->left  = recursiveReadDataBase(dataBase, newNode);
        if (!newNode->left) {
            treeDtor(newNode);
            return NULL;
        } else if (newNode->left == NODE_null) {
            newNode->left = NULL;
        }

        newNode->right = recursiveReadDataBase(dataBase, newNode);
        if (!newNode->right) {
            treeDtor(newNode);
            return NULL;
        } else if (newNode->right == NODE_null) {
            newNode->right = NULL;
        }

        return newNode;
    }

    fwscanf(dataBase, L" ; %n", &scannedCounter);
    if (scannedCounter != 0) {
        if (nullNode) return NODE_null;

        wlogPrint(L_EXTRA, 0, L"\tCurrent token is leaf\n");
        return newNode;
    } else {
        wlogPrint(L_ZERO, 1, L"[Wrong format]Expected ; or : got neither\n");
        treeDtor(newNode);
        return NULL;
    }
}


enum akinatorStatus akinatorInit(const char *dataBaseFile, Akinator_t *akinator) {
    char buffer[AKINATOR_BUFFER_SIZE] = ""; //internal buffer for string manipulations
    sprintf(buffer, "%s/%s", AKINATOR_DATA_DIR, dataBaseFile);
    logPrint(L_DEBUG, 0, "Initializing akinator\n"
                         "Trying to read dataBase from '%s'\n", buffer);

    struct stat stBuf = {0};
    if (stat(buffer, &stBuf) != 0) {
        logPrint(L_ZERO, 1, "Failed to read dataBase from '%s'\n"
                            "Akinator will use empty dataBase\n", buffer);

        akinator->root = nodeCtor(L"Неизвестно что", NULL, sizeof(wchar_t) * MAX_LABEL_LEN);
        akinator->current = akinator->root;
        logPrint(L_DEBUG, 0, "Successfully constructed empty akinator[%p]: root[%p]\n", akinator, akinator->root);
        return AKINATOR_SUCCESS;
    }

    FILE *dataBase = fopen(buffer, "rb");
    node_t *dataBaseRoot = recursiveReadDataBase(dataBase, NULL);
    if (dataBaseRoot == NULL) {
        logPrint(L_ZERO, 1, "Database has wrong format, can't read\n");
        return AKINATOR_ERROR;
    }
    akinator->root = akinator->current = dataBaseRoot;
    fclose(dataBase);
    if (getLogLevel() >= L_DEBUG) treeDump(dataBaseRoot, sPrint);
    return AKINATOR_SUCCESS;
}

enum akinatorStatus akinatorPlay(Akinator_t *akinator) {
    return AKINATOR_SUCCESS;
}

enum akinatorStatus akinatorDelete(Akinator_t *akinator) {
    return AKINATOR_SUCCESS;
}


