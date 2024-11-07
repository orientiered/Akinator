#include <stdlib.h>
#include <stdio.h>

#include "error_debug.h"
#include "logger.h"
#include "tree.h"

int PrintInt(char *buffer, const void* a) {
    return sprintf(buffer, "%d", *(const int *)a);
}

int main() {
    logOpen("log.txt", L_HTML_MODE);

    int startValue = 10;
    node_t *root = nodeCtor(&startValue, NULL, sizeof(int));
    startValue += 10;
    treeAdd(root, &startValue, 0);
    startValue += 10;
    node_t *node1 = treeAdd(root, &startValue, 1);
    treeAdd(node1, &startValue, 1);
    treeDump(root, PrintInt);

    treeDtor(root);
    logClose();
    return 0;
}
