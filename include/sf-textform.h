#ifndef SF_TEXT_FORM_H
#define SF_TEXT_FORM_H

const size_t MAX_INPUT_LEN = 63;
const wchar_t BACKSPACE_SYMBOL = 8;

typedef struct {
    sf::RenderWindow *window;
    sf::RectangleShape box;

    wchar_t inputStr[MAX_INPUT_LEN+1];
    size_t inputSize;
    sf::Text label;

    bool isSelected;
    bool visible;
} TextForm_t;

void textFormCtor(TextForm_t *form, sf::RenderWindow *window, sf::Font *font, sf::Vector2f pos, sf::Vector2f size);

void textFormSetVisible(TextForm_t *form, bool visible);

const wchar_t *textFormGetText(TextForm_t *form);
void textFormClear(TextForm_t *form);

void textFormUpdate(TextForm_t *form, wchar_t symbol);
void textFormClickEventUpdate(TextForm_t *form);

void textFormDraw(TextForm_t *form);

void textFormDtor(TextForm_t *form);

#endif
