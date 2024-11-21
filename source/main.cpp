#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <wchar.h>
#include <locale.h>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "sf-button.h"
#include "sf-textform.h"

#include "error_debug.h"
#include "logger.h"
#include "tree.h"

#include "jsonParser.h"

#include "akinator.h"
#include "tts.h"

int sPrintInt(void *buffer, const void* a);
int sPrintDouble(void *buffer, const void* a);
void binTreeTest();

int main() {
    json_t *config = jsonParseFromFile("assets/config.json", NULL);

    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Akinator", sf::Style::Fullscreen);
    sf::Font font;
    font.loadFromFile("assets/NotoSerifTC-VariableFont_wght.ttf");

    logOpen("log.txt", L_HTML_MODE);
    setLogLevel(L_EXTRA);
    logDisableBuffering();
    Akinator_t akinator = {};
    if (akinatorInit(&akinator, config, "dataBase.tdf", &window, &font) != AKINATOR_SUCCESS)
        return 1;
    akinatorPlay(&akinator);
    akinatorDelete(&akinator);

    jsonDtor(config);
    logClose();
    return 0;
}


int sPrintInt(void *buffer, const void* a) {
    return swprintf((wchar_t*)buffer, MAX_LABEL_LEN, L"%d", *(const int *)a);
}
int sPrintDouble(void *buffer, const void* a) {
    return swprintf((wchar_t*)buffer, MAX_LABEL_LEN, L"%.3g", *(const double *)a);
}
int cmpDouble(const void *a, const void *b) {
    double da = *(const double *)a,
           db = *(const double *)b;
    return (da-db) < 0 ? -1 : 1;
}

void binTreeTest() {
     int startValue = 10;
    node_t *root = nodeCtor(&startValue, NULL, sizeof(int));
    startValue += 10;
    treeAdd(root, &startValue, 0);
    startValue += 10;
    node_t *node1 = treeAdd(root, &startValue, 1);
    treeAdd(node1, &startValue, 1);
    treeDump(root, sPrintInt);

    treeDtor(root);

    double hehe = 5.02;
    root = nodeCtor(&hehe, NULL, sizeof(double));
    for (int i =0; i < 100; i++) {
        double temp = (double)rand() / RAND_MAX;
        logPrint(L_ZERO, 1, "Inserting %g\n", temp);
        treeInsert(root, &temp, cmpDouble);
    }
    treeDump(root, sPrintDouble);
    treeDtor(root);
}
