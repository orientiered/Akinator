#ifndef AKINATOR_FILE_IO_H
#define AKINATOR_FILE_IO_H

enum akinatorStatus readDatabaseFromFile(FILE *dataBase, Akinator_t *akinator);

enum akinatorStatus akinatorSaveDataBase(Akinator_t *akinator);

#endif
