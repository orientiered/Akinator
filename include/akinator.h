#ifndef AKINATOR_H
#define AKINATOR_H

#define AKINATOR_DUMP_DOT_FORMAT "akinator_%04zu.dot"
#define AKINATOR_DUMP_IMG_FORMAT "akinator_dump_%04zu."

const size_t MAX_LABEL_LEN = 128;
const size_t AKINATOR_BUFFER_SIZE = 256;
const char * const AKINATOR_DATA_DIR = "data";
const char * const defaultDatabaseFile = "database.tdf";

const wchar_t * const NULL_NODE_STRING = L"_null";

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

enum akinatorStatus akinatorInit(Akinator_t *akinator, json_t *config, const char *dataBaseFile, sf::RenderWindow *window, sf::Font *font);
enum akinatorStatus akinatorPlay(Akinator_t *akinator);
enum akinatorStatus akinatorDelete(Akinator_t *akinator);
size_t akinatorDump(Akinator_t *akinator, node_t *highlight);

#endif
