#ifndef AKINATOR_H
#define AKINATOR_H

const size_t MAX_LABEL_LEN = 128;
const size_t AKINATOR_BUFFER_SIZE = 256;
const char *AKINATOR_DATA_DIR = "data";

enum akinatorStatus {
    AKINATOR_SUCCESS,
    AKINATOR_ERROR
};

typedef struct {
    node_t *root;
    node_t *current;

} Akinator_t;

enum akinatorStatus akinatorInit(const char *dataBaseFile, Akinator_t *akinator);
enum akinatorStatus akinatorPlay(Akinator_t *akinator);
enum akinatorStatus akinatorDelete(Akinator_t *akinator);

#endif
