#ifndef AKINATOR_H
#define AKINATOR_H

#define AKINATOR_DUMP_DOT_FORMAT "akinator_%04zu.dot"
#define AKINATOR_DUMP_IMG_FORMAT "akinator_dump_%04zu."

const size_t MAX_LABEL_LEN = 128;
const size_t AKINATOR_BUFFER_SIZE = 256;
const char * const AKINATOR_DATA_DIR = "data";
const char * const defaultDatabaseFile = "database.tdf";

const wchar_t * const NULL_NODE_STRING = L"_null";

const wchar_t * const YES_STRINGS[] = {L"YES", L"Y", L"ДА", L"АГА", L"Конечно", L"Кнчн"};
const wchar_t * const NO_STRINGS[]  = {L"NO", L"Нет", L"Не", L"Nope", L"N"};
const wchar_t * const WELCOME_MSG_FORMAT_STR    = L"Привет, я Акинатор. Придумайте какую-нибудь хрень, а я её отгадаю";
const wchar_t * const AGREEMENTS_FORMAT_STR     = L"Чтобы согласиться со мной, используйте следующие слова (регистр не важен):";
const wchar_t * const DISAGREEMENTS_FORMAT_STR  = L"Чтобы не согласиться, используйте следующие слова (регистр не важен):";
const wchar_t * const QUESTION_FORMAT_STR       = L"Загаданный объект %ls? (Y/n)";
const wchar_t * const ADD_OBJECT_FORMAT_STR     = L"У меня нет идей. Что же вы загадали?";
const wchar_t * const OBJECT_DIFFER_FORMAT_STR  = L"Бро, чем %ls отличается от %ls?";
const wchar_t * const BAD_INPUT_FORMAT_STR      = L"Какой-то неровный базар у тебя. Ответь нормально -_-";
const wchar_t * const CORRECT_GUESS_FORMAT_STR  = L"Очев\n";
const wchar_t * const PLAY_AGAIN_FORMAT_STR     = L"Хотите сыграть ещё? (Y/n)";
const wchar_t * const SAVE_DATA_FORMAT_STR      = L"Сохранить прогресс? (Y/n)";

const wchar_t * const SUBMIT_BUTTON_LABEL       = L"Отправить";
const wchar_t * const YES_BUTTON_LABEL          = L"Да";
const wchar_t * const  NO_BUTTON_LABEL          = L"Нет";
const wchar_t * const GIVE_DEFINITION_LABEL     = L"Дать определение";
const wchar_t * const GIVE_COMPARISON_LABEL     = L"Сравнить объекты";
const wchar_t * const GIVE_DEFINITION_SCAN_STR  = L" Шо такое \"%l[^\n\"]\"";
const wchar_t * const COMPARE_SCAN_STR          = L" Сравни%n";
const wchar_t * const NO_LABEL_FORMAT_STR       = L"Я не знаю, что такое %ls";
const wchar_t * const COMPARE_FORMAT_STR        = L"%ls отличается от %ls тем, что ";
const wchar_t * const COMPARE_SIMILAR_FORMAT_STR= L"\nИх общие черты ";

enum choiceButtonsState {
    CLICKED_YES =  1,
    CLICKED_NO  = -1,
    NOT_CLICKED =  0
};

enum akinatorState {
    STATE_QUESTION,
    STATE_ADD_NEW_OBJECT,
    STATE_ADD_NEW_QUESTION,
    STATE_ASK_PLAY_AGAIN,
    STATE_ASK_SAVE_DATABASE,
    STATE_DEF_COMP_MODE,
    STATE_END_EXECUTION
};

enum akinatorStatus {
    AKINATOR_SUCCESS,
    AKINATOR_ERROR,
    AKINATOR_READ_ERROR
};

typedef struct {
    sf::Music bkgMusic;

    sf::RenderWindow *window;
    sf::Font *font;

    Button_t buttonYes;
    Button_t buttonNo;

    Button_t buttonNextMode;    ///< Changes akinator mode guess->definition->comparison

    Button_t questionBox;       ///< Not actually a button, just textbox where all question are written
    TextForm_t inputForm;       ///< Main input Form
    TextForm_t inputForm_b;     ///< Appears only in comparison mode

    sf::Texture dumpTexture;
    sf::Sprite  dumpImg;

    sf::Texture backgroundTexture;
    sf::Sprite  backgroundImg;
} GUILayout_t;

typedef struct {
    GUILayout_t gui;
    char *databaseFile;

    enum akinatorState  state;
    //enum responseStatus playerResponse;
    bool isRunning;
    node_t *root;

    node_t *previous;
    node_t *current;
} Akinator_t;

enum akinatorStatus akinatorInit(Akinator_t *akinator, const char *dataBaseFile, sf::RenderWindow *window, sf::Font *font);
enum akinatorStatus akinatorPlay(Akinator_t *akinator);
enum akinatorStatus akinatorDelete(Akinator_t *akinator);
size_t akinatorDump(Akinator_t *akinator, node_t *highlight);

#endif
