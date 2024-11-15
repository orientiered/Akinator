#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//? Could this include be removed?
#include <SFML/Graphics.hpp>
#include "sf-button.h"
#include "sf-textform.h"

#include "logger.h"
#include "tree.h"
#include "akinator.h"
#include "akinatorFileIO.h"

static node_t *NODE_null = (node_t *) size_t(-1);

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

enum akinatorStatus readDatabaseFromFile(FILE *dataBase, Akinator_t *akinator) {
    MY_ASSERT(dataBase, exit(1));
    node_t *root = recursiveReadDataBase(dataBase, NULL);
    if (!root) return AKINATOR_READ_ERROR;
    akinator->root = akinator->current = root;
    akinator->previous = NULL;
    return AKINATOR_SUCCESS;
}

static enum akinatorStatus recursiveSaveDataBase(FILE *file, node_t *node, unsigned tabulation) {
    MY_ASSERT(file, exit(1));

    for (unsigned i = 0; i < tabulation; i++) fputwc(L'\t', file);
    if (!node) {
        fwprintf(file, L"\"%ls\";\n", NULL_NODE_STRING);
        return AKINATOR_SUCCESS;
    }

    fwprintf(file, L"\"%ls\"", (wchar_t *)node->data);
    if (!node->left && !node->right) {
        fwprintf(file, L";\n");
        return AKINATOR_SUCCESS;
    }

    if (node->left || node->right) fwprintf(file, L":\n");

    recursiveSaveDataBase(file, node->left,  tabulation+1);
    recursiveSaveDataBase(file, node->right, tabulation+1);

    return AKINATOR_SUCCESS;
}

enum akinatorStatus akinatorSaveDataBase(Akinator_t *akinator) {
    MY_ASSERT(akinator, exit(1));

    if (!akinator->databaseFile) {
        strcpy(akinator->databaseFile, defaultDatabaseFile);
        logPrint(L_ZERO, 1, "[Error]No output file name, using default name\n");

    }
    FILE *database = fopen(akinator->databaseFile, "wb");
    if (!database) {
        logPrint(L_ZERO, 1, "[Error]Failed to open database file '%s'\n", akinator->databaseFile);
        return AKINATOR_ERROR;
    }

    if (recursiveSaveDataBase(database, akinator->root, 0) != AKINATOR_SUCCESS) {
        logPrint(L_ZERO, 1, "[Error]Failed to save database\n");
        return AKINATOR_ERROR;
    }

    fclose(database);
    return AKINATOR_SUCCESS;
}
