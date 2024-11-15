#ifndef SF_BUTTON_H
#define SF_BUTTON_H

#include <stdint.h>

const sf::Color BUTTON_MAIN_COLOR   (0xFFFFFF55);
const sf::Color BUTTON_HOVER_COLOR  (0xEEEEEE55);
const sf::Color BUTTON_PRESSED_COLOR(0x88888855);

typedef struct {
    sf::RenderWindow *window;
    sf::RectangleShape box;
    sf::Text label;

    bool isPressed;
    bool visible;
} Button_t;

void buttonCtor(Button_t *button, sf::RenderWindow *window, sf::Font *font,
                const wchar_t *label, sf::Vector2f pos, sf::Vector2f size);

void buttonSetVisible(Button_t *button, bool visible);

void buttonSetLabel(Button_t *button, const wchar_t *label);

void buttonUpdate(Button_t *button);
bool buttonClickEventUpdate(Button_t *button);

void buttonDraw(Button_t *button);

void buttonDtor(Button_t *button);

#endif
