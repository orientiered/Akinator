#ifndef AKINATOR_H
#define AKINATOR_H

const size_t MAX_LABEL_LEN = 128;
const size_t AKINATOR_BUFFER_SIZE = 256;
const char * const AKINATOR_DATA_DIR = "data";
const char * const defaultDatabaseFile = "database.tdf";

const wchar_t * const NULL_NODE_STRING = L"_null";

const wchar_t * const YES_STRINGS[] = {L"YES", L"Y", L"ДА", L"АГА", L"Конечно", L"Кнчн"};
const wchar_t * const NO_STRINGS[]  = {L"NO", L"Нет", L"Не", L"Nope", L"N"};
const wchar_t * const WELCOME_MSG_FORMAT_STR    = L"Привет, я Акинатор. Придумайте какую-нибудь хрень, а я её отгадаю\n";
const wchar_t * const AGREEMENTS_FORMAT_STR     = L"Чтобы согласиться со мной, используйте следующие слова (регистр не важен):";
const wchar_t * const DISAGREEMENTS_FORMAT_STR  = L"Чтобы не согласиться, используйте следующие слова (регистр не важен):";
const wchar_t * const QUESTION_FORMAT_STR       = L"Загаданный объект %ls? (Y/n)\n";
const wchar_t * const ADD_OBJECT_FORMAT_STR     = L"У меня нет идей. Что же вы загадали?\n";
const wchar_t * const OBJECT_DIFFER_FORMAT_STR  = L"Бро, чем %ls отличается от %ls?\n";
const wchar_t * const BAD_INPUT_FORMAT_STR      = L"Какой-то неровный базар у тебя. Ответь нормально -_-\n";
const wchar_t * const CORRECT_GUESS_FORMAT_STR  = L"Очев\n";
const wchar_t * const PLAY_AGAIN_FORMAT_STR     = L"Хотите сыграть ещё? (Y/n)\n";
const wchar_t * const SAVE_DATA_FORMAT_STR      = L"Сохранить прогресс? (Y/n)\n";

const wchar_t * const GIVE_DEFINITION_SCAN_STR  = L" Шо такое \"%l[^\n\"]\"";
const wchar_t * const NO_LABEL_FORMAT_STR       = L"Я не знаю, что такое %ls\n";
enum requestType {
    REQUEST_YES_NO,
    REQUEST_STRING,
};

enum responseStatus {
    RESPONSE_SUCCESS_YES,
    RESPONSE_SUCCESS_NO,
    RESPONSE_SUCCESS_STRING,
    RESPONSE_SUCCESS_DEFINITION,
    RESPONSE_BAD_INPUT,
    RESPONSE_NO
};

enum akinatorStatus {
    AKINATOR_SUCCESS,
    AKINATOR_ERROR
};

typedef struct {
    char *databaseFile;

    node_t *root;
    node_t *current;
} Akinator_t;

enum akinatorStatus akinatorInit(const char *dataBaseFile, Akinator_t *akinator);
enum akinatorStatus akinatorPlay(Akinator_t *akinator);
enum akinatorStatus akinatorDelete(Akinator_t *akinator);

#endif
