#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#include "logger.h"
#include "utils.h"

#include "cList.h"
#include "tree.h"
#include "akinator.h"
#include "tts.h"

static node_t *NODE_null = (node_t *) size_t(-1);

static int sPrint(void *buffer, const void *a) {
    return swprintf((wchar_t*)buffer, MAX_LABEL_LEN, (const wchar_t*) a);
}
static int akinatorStrCmp(const void *a, const void *b) {
    return wcscasecmp((const wchar_t *) a, (const wchar_t *) b);
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

    char *fileName = (char *) calloc(strlen(buffer) + 1, sizeof(char));
    strcpy(fileName, buffer);
    akinator->databaseFile = fileName;

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

static bool matchAnyCaseString(const wchar_t *const word, const wchar_t * const *words, size_t wordsLen) {
    for (size_t idx = 0; idx < wordsLen; idx++) {
        if (wcscasecmp(word, words[idx]) == 0)
            return true;
    }
    return false;
}

static enum responseStatus getPlayerResponse(wchar_t *ansBuffer, enum requestType type) {
    enum responseStatus playerAnswer = RESPONSE_NO;
    while (playerAnswer == RESPONSE_BAD_INPUT || playerAnswer == RESPONSE_NO) {
        if (playerAnswer == RESPONSE_BAD_INPUT) {
            ttsPrintf(BAD_INPUT_FORMAT_STR);
            ttsFlush();
        }

        if (fwscanf(stdin, GIVE_DEFINITION_SCAN_STR, ansBuffer) == 1) {
            return RESPONSE_SUCCESS_DEFINITION;
        }

        fwscanf(stdin, L" %l[^\n]", ansBuffer);

        switch(type) {
        case REQUEST_YES_NO:
            if (matchAnyCaseString(ansBuffer, YES_STRINGS, ARRAY_SIZE(YES_STRINGS)))
                playerAnswer = RESPONSE_SUCCESS_YES;
            else if (matchAnyCaseString(ansBuffer, NO_STRINGS, ARRAY_SIZE(NO_STRINGS)))
                playerAnswer = RESPONSE_SUCCESS_NO;
            else playerAnswer = RESPONSE_BAD_INPUT;
            break;
        case REQUEST_STRING:
            //TODO: check if 'no' is in string
            playerAnswer = RESPONSE_SUCCESS_STRING;
            break;
        default:
            //MAYBE NOT 1
            exit(1);
            break;
        }
    }
    return playerAnswer;
}

static akinatorStatus akinatorWelcomeMessage() {
    ttsPrintf(WELCOME_MSG_FORMAT_STR);
    ttsFlush();
    wprintf(AGREEMENTS_FORMAT_STR);
    for (size_t idx = 0; idx < sizeof(YES_STRINGS) / sizeof(wchar_t *); idx++)
        wprintf(L"%ls ", YES_STRINGS[idx]);
    wprintf(L"\n");

    wprintf(DISAGREEMENTS_FORMAT_STR);
    for (size_t idx = 0; idx < sizeof(NO_STRINGS) / sizeof(wchar_t *); idx++)
        wprintf(L"%ls ", NO_STRINGS[idx]);
    wprintf(L"\n");

    return AKINATOR_SUCCESS;
}

static akinatorStatus akinatorGiveDefinition(Akinator_t *akinator, const wchar_t *ansBuffer) {
    node_t *label = treeFind(akinator->root, ansBuffer, akinatorStrCmp);
    if (!label) {
        ttsPrintf(NO_LABEL_FORMAT_STR, ansBuffer);
        ttsFlush();
        return AKINATOR_SUCCESS;
    }
    const size_t MAX_DEFINITION_DEPTH = 64;
    //TODO: maybe do it other way
    wchar_t *positiveProperties[MAX_DEFINITION_DEPTH] = {0};
    size_t positiveIdx = 0;
    wchar_t *negativeProperties[MAX_DEFINITION_DEPTH] = {0};
    size_t negativeIdx = 0;

    node_t *current = label->parent,
           *prev    = label;
    while (current != NULL) {
        if (current->right == prev)
            negativeProperties[negativeIdx++] = (wchar_t *) current->data;
        else
            positiveProperties[positiveIdx++] = (wchar_t *) current->data;

        prev = current;
        current = current->parent;
    }
    // positiveProperties[positiveIdx++] = (wchar_t *) current->data;

    ttsPrintf(L"%ls это ", label->data);

    for (size_t idx = 0; idx < positiveIdx; idx++) {
        ttsPrintf(L"%ls, ", positiveProperties[idx]);
    }
    for (size_t idx = 0; idx < negativeIdx; idx++) {
        ttsPrintf(L"не %ls, ", negativeProperties[idx]);
    }
    ttsPrintf(L"\n");
    ttsFlush();
    return AKINATOR_SUCCESS;
}

enum akinatorStatus akinatorPlay(Akinator_t *akinator) {
    wchar_t ansBuffer[AKINATOR_BUFFER_SIZE] = L"";
    akinatorWelcomeMessage();

    bool run = true;
    enum responseStatus playerAnswer = RESPONSE_BAD_INPUT;

    while (run) {
        ttsPrintf(QUESTION_FORMAT_STR, akinator->current->data);
        ttsFlush();
        playerAnswer = getPlayerResponse(ansBuffer, REQUEST_YES_NO);

        if (playerAnswer == RESPONSE_SUCCESS_DEFINITION) {
            akinatorGiveDefinition(akinator, ansBuffer);
            continue;
        }

        if (akinator->current->left) {
            if (playerAnswer == RESPONSE_SUCCESS_YES)
                akinator->current = akinator->current->left;
            else if (playerAnswer == RESPONSE_SUCCESS_NO)
                akinator->current = akinator->current->right;
        } else {
            if (playerAnswer == RESPONSE_SUCCESS_YES) {
                ttsPrintf(CORRECT_GUESS_FORMAT_STR);
            } else if (playerAnswer == RESPONSE_SUCCESS_NO) {
                ttsPrintf(ADD_OBJECT_FORMAT_STR);
                ttsFlush();
                playerAnswer = getPlayerResponse(ansBuffer, REQUEST_STRING);
                treeAdd(akinator->current, ansBuffer, 0);
                treeAdd(akinator->current, akinator->current->data, 1);

                ttsPrintf(OBJECT_DIFFER_FORMAT_STR, ansBuffer, akinator->current->data);
                ttsFlush();
                playerAnswer = getPlayerResponse(ansBuffer, REQUEST_STRING);
                wcscpy((wchar_t*)(akinator->current->data), ansBuffer);
            }

            ttsPrintf(PLAY_AGAIN_FORMAT_STR);
            ttsFlush();
            playerAnswer = getPlayerResponse(ansBuffer, REQUEST_YES_NO);
            akinator->current = akinator->root;
            run = (playerAnswer == RESPONSE_SUCCESS_YES) ? true : false;
        }
    }
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

static enum akinatorStatus akinatorSaveDataBase(Akinator_t *akinator) {
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
    return AKINATOR_SUCCESS;
}

enum akinatorStatus akinatorDelete(Akinator_t *akinator) {
    treeDump(akinator->root, sPrint);
    wprintf(SAVE_DATA_FORMAT_STR);
    wchar_t ansBuffer[MAX_LABEL_LEN] = L"";

    enum responseStatus playerAnswer = getPlayerResponse(ansBuffer, REQUEST_YES_NO);
    if (playerAnswer == RESPONSE_SUCCESS_YES) {
        if (akinatorSaveDataBase(akinator) != AKINATOR_SUCCESS)
            return AKINATOR_ERROR;
    }

    free(akinator->databaseFile);
    treeDtor(akinator->root);

    return AKINATOR_SUCCESS;
}


