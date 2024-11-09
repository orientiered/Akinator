#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "logger.h"

#include "tree.h"
#include "akinator.h"

static sPrint()
node_t *recursiveReadDataBase(FILE *dataBase, node_t *parent) {
    static wchar_t buffer[AKINATOR_BUFFER_SIZE] = L"";
    fwscanf(dataBase, " %![^\"]\"%[^\"]\"", buffer);
    wlogPrint(L_EXTRA, L"Akinator: readed token '%s'\n", buffer);
    node_t *newNode = nodeCtor(buffer, parent, sizeof(wchar_t) * MAX_LABEL_LEN);

}


enum akinatorStatus akinatorInit(const char *dataBaseFile, Akinator_t *akinator) {
    char buffer[AKINATOR_BUFFER_SIZE] = ""; //internal buffer for string manipulations
    logPrint(L_DEBUG, 0, "Initializing akinator\n"
                         "Trying to read dataBase from '%s'\n", dataBaseFile);

    sprintf(buffer, "%s/%s", AKINATOR_DATA_DIR, dataBaseFile);
    struct stat stBuf = {0};
    if (stat(buffer, &stBuf) != 0) {
        logPrint(L_ZERO, 1, "Failed to read dataBase from '%s'\n"
                            "Akinator will use empty dataBase\n", buffer);

        akinator->root = nodeCtor(L"Неизвестно что", NULL, sizeof(wchar_t) * MAX_LABEL_LEN);
        akinator->current = akinator->root;
        logPrint(L_DEBUG, 0, "Successfully constructed empty akinator[%p]: root[%p]\n", akinator, akinator->root);
        return AKINATOR_SUCCESS;
    }

    FILE *dataBase = fopen(buffer, "r");
    if (recursiveReadDataBase(dataBase, akinator) != AKINATOR_SUCCESS) {
        logPrint(L_ZERO, 1, "Database has wrong format, can't read\n");
        return AKINATOR_ERROR;
    }
    fclose(dataBase);
    if (getLogLevel() >= L_DEBUG) treeDump(root)
    return AKINATOR_SUCCESS;
}



