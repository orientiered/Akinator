#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>

#include "logger.h"

#include "sf-button.h"

//TODO you can click only one button at a time
static bool isAnyButtonPressed = false;

/// @brief convert from (0,1)^2 to (0, window.width)x(0, window.height)
static sf::Vector2f mapCoords(sf::Vector2f coords, const sf::RenderWindow *window) {
    sf::Vector2u windowSize = window->getSize();
    return sf::Vector2f(coords.x * windowSize.x, coords.y * windowSize.y);
}

/// @brief Center text in the box and set max font that fits inside it
/// @return Size of selected font
static unsigned fitTextToBox(sf::Text *text, const sf::RectangleShape *box, unsigned maxFontSize) {
    unsigned currentFontSize = maxFontSize + 1;
    sf::Vector2f textSize = {0, 0};

    do {
        currentFontSize--;
        text->setCharacterSize(currentFontSize);
        textSize.x = text->getGlobalBounds().width;
        textSize.y = text->getGlobalBounds().height;
    } while (textSize.x > box->getSize().x || textSize.y > box->getSize().y);

    // origin of box is it's center
    sf::Vector2f boxCenter = box->getPosition();
    text->setPosition(boxCenter - textSize * 0.5f);

    return currentFontSize;
}

void buttonCtor(Button_t *button, sf::RenderWindow *window, sf::Font *font,
                const wchar_t *label, sf::Vector2f pos, sf::Vector2f size) {
    button->window = window;

    button->box.setSize(mapCoords(size, window));
    button->box.setOrigin(mapCoords(size * 0.5f, window));
    button->box.setPosition(mapCoords(pos, window));
    button->box.setFillColor(BUTTON_MAIN_COLOR);

    button->label.setFont(*font);
    button->label.setCharacterSize(100);
    button->label.setString(label);
    button->label.setStyle(sf::Text::Bold);
    button->label.setFillColor(sf::Color(0xFFFF00FF));
    // button->label.setOrigin(mapCoords(size * 0.5f, window));
    fitTextToBox(&button->label, &button->box, BUTTON_MAX_FONT_SIZE);

    button->visible = true;
}

void buttonSetVisible(Button_t *button, bool visible) {
    button->visible = visible;
    if (visible)
        button->box.setFillColor(BUTTON_MAIN_COLOR);
}

void buttonSetLabel(Button_t *button, const wchar_t *label) {
    button->label.setString(label);
    fitTextToBox(&button->label, &button->box, BUTTON_MAX_FONT_SIZE);
}

void buttonUpdate(Button_t *button) {
    if (!button->visible)
        return;

    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(*button->window));
    // bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    bool mouseInBox = button->box.getGlobalBounds().contains(mousePos);
    if (mouseInBox) {
        if (button->isPressed) {
            button->box.setFillColor(BUTTON_PRESSED_COLOR);
        } else
            button->box.setFillColor(BUTTON_HOVER_COLOR);
    } else {
        button->isPressed = false;
        button->box.setFillColor(BUTTON_MAIN_COLOR);
    }
}

bool buttonClickEventUpdate(Button_t *button) {
    if (!button->visible)
        return false;

    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(*button->window));
    bool mouseInBox = button->box.getGlobalBounds().contains(mousePos);
    bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    if (mouseInBox && !mousePressed && button->isPressed) {
        wlogPrint(L_DEBUG, 0, L"Button [%p] pressed+released\n", button);
        button->isPressed = false;
        return true;
    } else if (mouseInBox && mousePressed) {
        logPrint(L_DEBUG, 0, "Button [%p] pressed\n", button);
        button->isPressed = true;
        return false;
    }
    return false;
}

void buttonDraw(Button_t *button) {
    if (!button->visible)
        return;

    button->window->draw(button->box);
    button->window->draw(button->label);
}

void buttonDtor(Button_t *button) {
    logPrint(L_EXTRA, 0, "Deconstructed button[%p]\n", button);
}
