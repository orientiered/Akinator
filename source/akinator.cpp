#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#include <SFML/Graphics.hpp>

#include "logger.h"
#include "utils.h"

#include "cList.h"
#include "tree.h"
#include "akinator.h"
#include "tts.h"

static node_t *NODE_null = (node_t *) size_t(-1);

static int akinatorSWPrint(void *buffer, const void *a) {
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


enum akinatorStatus akinatorInit(Akinator_t *akinator, const char *dataBaseFile, sf::RenderWindow *window, sf::Font *font) {
    //TODO: check input parameters
    akinator->window = window;
    akinator->font   = font;

    akinator->isRunning = true;
    akinator->playerResponse = RESPONSE_NO;

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

        const wchar_t *rootNodeLabel = L"Неизвестно что";
        akinator->root = nodeCtor(rootNodeLabel, NULL, sizeof(wchar_t) * (wcslen(rootNodeLabel) + 1));
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
    if (getLogLevel() >= L_DEBUG) treeDump(dataBaseRoot, akinatorSWPrint);
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

        size_t scannedChars = 0;
        if (wscanf(GIVE_DEFINITION_SCAN_STR, ansBuffer) == 1) {
            return RESPONSE_SUCCESS_DEFINITION;
        } else if (wscanf(COMPARE_SCAN_STR, &scannedChars) >= 0 && scannedChars > 0) {
            return RESPONSE_SUCCESS_COMPARE;
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

typedef struct {
    wchar_t label[MAX_LABEL_LEN];
    bool negative;
} objProperty_t;

static bool isEqualObjProperty(const objProperty_t *a, const objProperty_t *b) {
    return (a->negative == b->negative) && wcscasecmp(a->label, b->label) == 0;
}

static objProperty_t *createDefinition(Akinator_t *akinator, node_t *object) {
    MY_ASSERT(akinator, exit(1));
    MY_ASSERT(object, exit(1));
    const size_t MAX_DEFINITION_DEPTH = 64;
    objProperty_t *definition = (objProperty_t *) calloc(MAX_DEFINITION_DEPTH, sizeof(objProperty_t));
    size_t defIdx = 0;
    node_t *curr = object->parent,
           *prev = object;

    while (curr != NULL) {
        if (curr->right == prev)
            definition[defIdx].negative = true;
        wcscpy(definition[defIdx].label, (wchar_t *)curr->data);
        defIdx++;
        prev = curr; curr = curr->parent;
    }

    reverseArray(definition, sizeof(objProperty_t), defIdx);
    return definition;
}

static akinatorStatus printDefinition(objProperty_t *definition) {
    MY_ASSERT(definition, exit(1));
    for (size_t idx = 0; definition[idx].label[0]; idx++) {
        if (definition[idx].negative)
            ttsPrintf(L"не ");
        ttsPrintf(L"%ls", definition[idx].label);
        if (definition[idx+1].label[0])
            ttsPrintf(L", ");
    }
    return AKINATOR_SUCCESS;
}

static akinatorStatus akinatorGiveDefinition(Akinator_t *akinator, const wchar_t *ansBuffer) {
    node_t *label = treeFind(akinator->root, ansBuffer, akinatorStrCmp);
    if (!label) {
        ttsPrintf(NO_LABEL_FORMAT_STR, ansBuffer);
        ttsFlush();
        return AKINATOR_SUCCESS;
    }

    objProperty_t *definition = createDefinition(akinator, label);
    ttsPrintf(L"%ls это ", label->data);
    printDefinition(definition);

    free(definition);
    ttsPrintf(L"\n");
    ttsFlush();
    return AKINATOR_SUCCESS;
}

static enum akinatorStatus akinatorCompare(Akinator_t *akinator, wchar_t *ansBuffer) {
    MY_ASSERT(akinator, exit(1));
    MY_ASSERT(ansBuffer, exit(1));

    while (getPlayerResponse(ansBuffer, REQUEST_STRING) != RESPONSE_SUCCESS_STRING)
        ;
    node_t *firstLabel = treeFind(akinator->root, ansBuffer, akinatorStrCmp);
    if (!firstLabel) {
        wprintf(NO_LABEL_FORMAT_STR, ansBuffer);
        return AKINATOR_SUCCESS;
    }

    while (getPlayerResponse(ansBuffer, REQUEST_STRING) != RESPONSE_SUCCESS_STRING)
        ;
    node_t *secondLabel = treeFind(akinator->root, ansBuffer, akinatorStrCmp);
    if (!secondLabel) {
        wprintf(NO_LABEL_FORMAT_STR, ansBuffer);
        return AKINATOR_SUCCESS;
    }

    objProperty_t *firstDef  = createDefinition(akinator, firstLabel);
    objProperty_t *secondDef = createDefinition(akinator, secondLabel);
    size_t idx = 0;
    while (firstDef[idx].label[0] && isEqualObjProperty(firstDef + idx, secondDef + idx))
        idx++;

    ttsPrintf(COMPARE_FORMAT_STR, firstLabel->data, secondLabel->data);
    ttsPrintf(L"%ls ", firstLabel->data);
    printDefinition(firstDef + idx);

    ttsPrintf(L", а %ls ", secondLabel->data);
    printDefinition(secondDef + idx);

    if (idx != 0) {
        ttsPrintf(COMPARE_SIMILAR_FORMAT_STR);
        firstDef[idx].label[0] = L'\0';
        printDefinition(firstDef);
    }

    ttsPrintf(L"\n");
    ttsFlush();
    free(firstDef);
    free(secondDef);
    return AKINATOR_SUCCESS;
}

static enum akinatorStatus akinatorHandleQuestion(Akinator_t *akinator) {
    if (akinator->playerResponse == RESPONSE_SUCCESS_YES) {
        akinator->current = akinator->current->left;
        return AKINATOR_SUCCESS;
    }
    else if (akinator->playerResponse == RESPONSE_SUCCESS_NO) {
        akinator->current = akinator->current->right;
        return AKINATOR_SUCCESS;
    }
    LOG_PRINT(L_ZERO, 1, "Wrong response type: %d\n", akinator->playerResponse);
    return AKINATOR_ERROR;
}

static enum akinatorStatus akinatorHandleLeaf(Akinator_t *akinator, wchar_t *ansBuffer) {
    if (akinator->playerResponse == RESPONSE_SUCCESS_YES) {
        ttsPrintf(CORRECT_GUESS_FORMAT_STR);
    } else if (akinator->playerResponse == RESPONSE_SUCCESS_NO) {
        ttsPrintf(ADD_OBJECT_FORMAT_STR);
        ttsFlush();
        akinator->playerResponse = getPlayerResponse(ansBuffer, REQUEST_STRING);
        treeAdd(akinator->current, ansBuffer, 0);
        treeAdd(akinator->current, akinator->current->data, 1);

        ttsPrintf(OBJECT_DIFFER_FORMAT_STR, ansBuffer, akinator->current->data);
        ttsFlush();
        akinator->playerResponse = getPlayerResponse(ansBuffer, REQUEST_STRING);
        wcscpy((wchar_t*)(akinator->current->data), ansBuffer);
    }

    ttsPrintf(PLAY_AGAIN_FORMAT_STR);
    ttsFlush();
    akinator->playerResponse = getPlayerResponse(ansBuffer, REQUEST_YES_NO);
    akinator->current = akinator->root;
    akinator->isRunning = (akinator->playerResponse == RESPONSE_SUCCESS_YES);

    return AKINATOR_SUCCESS;
}

enum akinatorStatus akinatorPlay(Akinator_t *akinator) {
    MY_ASSERT(akinator, exit(1));

    wchar_t ansBuffer[AKINATOR_BUFFER_SIZE] = L"";
    akinatorWelcomeMessage();

    sf::RenderWindow *window = akinator->window;
    sf::Vector2f windowSize = sf::Vector2f(window->getSize());

    sf::Vector2f boxSize = sf::Vector2f(windowSize.x * 0.4, windowSize.y * 0.1);
    sf::RectangleShape currentNodeBox(boxSize);
    currentNodeBox.setOrigin(boxSize * 0.5f);
    currentNodeBox.setPosition(windowSize * 0.5f);

    sf::Text nodeText(L"Неизвестно что", *(akinator->font));
    nodeText.setFillColor(sf::Color::Black);
    nodeText.setPosition(windowSize * 0.5f + sf::Vector2f(-nodeText.getGlobalBounds().width * 0.5, 0));

    sf::Texture akinatorDumpTexture;
    akinatorDumpTexture.create(10, 10);
    sf::Sprite akinatorDumpImg;
    char dumpFileName[64] = "";
    akinatorDumpImg.setTexture(akinatorDumpTexture, true);

    while (akinator->isRunning && window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window->close();
        }
        if (!window->isOpen()) break;
        size_t imgNumber = akinatorDump(akinator, akinator->current);
        sprintf(dumpFileName, "logs/img/" AKINATOR_DUMP_IMG_FORMAT "png", imgNumber);

        logPrint(L_DEBUG, 0, "Loading img %s\n", dumpFileName);
        if (!akinatorDumpTexture.loadFromFile(dumpFileName)) {
            logPrint(L_ZERO, 1, "Can't load img %s\n", dumpFileName);
        }
        akinatorDumpImg.setScale(sf::Vector2f(1.5, 1.5));
        akinatorDumpImg.setTexture(akinatorDumpTexture, true);
        logPrint(L_DEBUG, 0, "Loaded img %dx%d\n", akinatorDumpTexture.getSize().x, akinatorDumpTexture.getSize().y);

        swprintf(ansBuffer, MAX_LABEL_LEN, QUESTION_FORMAT_STR, akinator->current->data);
        nodeText.setString(ansBuffer);
        nodeText.setPosition(windowSize * 0.5f + sf::Vector2f(-nodeText.getGlobalBounds().width * 0.5, 0));

        window->clear();
        window->draw(currentNodeBox);
        window->draw(nodeText);
        window->draw(akinatorDumpImg);
        window->display();

        ttsPrintf(QUESTION_FORMAT_STR, akinator->current->data);
        ttsFlush();
        akinator->playerResponse = getPlayerResponse(ansBuffer, REQUEST_YES_NO);

        if (akinator->playerResponse == RESPONSE_SUCCESS_DEFINITION) {
            akinatorGiveDefinition(akinator, ansBuffer);
            continue;
        } else if (akinator->playerResponse == RESPONSE_SUCCESS_COMPARE) {
            akinatorCompare(akinator, ansBuffer);
            continue;
        }

        if (akinator->current->left) {
            akinatorHandleQuestion(akinator);
        } else {
            akinatorHandleLeaf(akinator, ansBuffer);
        }
    }
    window->close();
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

    fclose(database);
    return AKINATOR_SUCCESS;
}

enum akinatorStatus akinatorDelete(Akinator_t *akinator) {
    treeDump(akinator->root, akinatorSWPrint);
    wprintf(SAVE_DATA_FORMAT_STR);
    wchar_t ansBuffer[MAX_LABEL_LEN] = L"";

    enum responseStatus playerAnswer = getPlayerResponse(ansBuffer, REQUEST_YES_NO);
    if (playerAnswer == RESPONSE_SUCCESS_YES) {
        if (akinatorSaveDataBase(akinator) != AKINATOR_SUCCESS)
            return AKINATOR_ERROR;
    }

    akinatorDump(akinator, NULL);
    free(akinator->databaseFile);
    treeDtor(akinator->root);

    return AKINATOR_SUCCESS;
}

//TODO: change function name
size_t akinatorDump(Akinator_t *akinator, node_t *highlight) {
    MY_ASSERT(akinator, exit(0));
    static size_t dumpCounter = 0;
    dumpCounter++;
    system("mkdir -p logs/dot logs/img");
    const char *QUESTION_YES_COLOR = "#10EE88";
    const char *QUESTION_NO_COLOR  = "#EE1088";
    const char *LEAF_YES_COLOR     = "#10EE10";
    const char *LEAF_NO_COLOR      = "#EE1010";
    const char *HIGHLIGHT_COLOR    = "#fffb00";

    char buffer[128] = "";
    sprintf(buffer, "logs/dot/" AKINATOR_DUMP_DOT_FORMAT, dumpCounter);
    FILE *dotFile = fopen(buffer, "wb");
    fwprintf(dotFile, L"digraph {\n"
                      L"graph [splines=line]\n");

    cList_t toVisit = {0};
    listCtor(&toVisit, sizeof(node_t *), NULL);
    listPushFront(&toVisit, &akinator->root);

    wchar_t valueBuffer[128] = L"";
    while (toVisit.size > 0) {
        node_t *current = *(node_t **)listGet(&toVisit, listFront(&toVisit));
        listPopFront(&toVisit);

        if (current->right)
            listPushFront(&toVisit, &(current->right));
        if (current->left)
            listPushFront(&toVisit, &(current->left));
        bool leaf      = (current->right || current->left);
        bool yes_node  = (current->parent == NULL || current->parent->left == current);

        const char *color = (current == highlight) ? HIGHLIGHT_COLOR :
                            ( leaf  &&  yes_node)  ? LEAF_YES_COLOR  :
                            ( leaf  && !yes_node)  ? LEAF_NO_COLOR   :
                            (!leaf  &&  yes_node)  ? QUESTION_YES_COLOR :
                            QUESTION_NO_COLOR;

        akinatorSWPrint(valueBuffer, current->data);
        fwprintf(dotFile, L"\tnode%p [shape = rectangle, style=filled,label = \"%ls\", fillcolor = \"%s\"];\n",
                                current,                        valueBuffer,       color);

        if (current->left)
            fwprintf(dotFile, L"\tnode%p -> node%p;\n", current, current->left);
        if (current->right)
            fwprintf(dotFile, L"\tnode%p -> node%p;\n", current, current->right);
    }
    listDtor(&toVisit);
    fwprintf(dotFile, L"}\n");
    fclose(dotFile);

    const char *extension = "svg";
    sprintf(buffer, "dot logs/dot/"AKINATOR_DUMP_DOT_FORMAT" -T%s -o logs/img/"AKINATOR_DUMP_IMG_FORMAT"%s",
                                        dumpCounter,        extension,            dumpCounter,      extension);
    system(buffer);
    logPrint(L_ZERO, 0, "<img src=\"img/" AKINATOR_DUMP_IMG_FORMAT "%s\" width=76%%>\n<hr>\n",
                                            dumpCounter,         extension);

    extension = "png";
    sprintf(buffer, "dot logs/dot/"AKINATOR_DUMP_DOT_FORMAT" -T%s -o logs/img/"AKINATOR_DUMP_IMG_FORMAT"%s",
                                        dumpCounter,        extension,            dumpCounter,      extension);
    system(buffer);

    return dumpCounter;
}

