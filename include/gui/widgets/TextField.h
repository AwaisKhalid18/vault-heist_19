#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "gui/widgets/RoundedRect.h"

using namespace std;

// A small input box for typing an integer. Click to focus, type digits
// (and a leading minus), Backspace deletes, Enter submits.
class TextField
{
public:
    TextField();

    void init(sf::Font* font, const string& placeholder, float x, float y, float width, float height);
    bool handleEvent(const sf::Event& e);   // true when Enter was pressed inside
    void update(float dt);
    void draw(sf::RenderWindow& window);

    bool getInt(int& out);                  // false if empty or not a number
    void clearText();
    void setFocused(bool on);
    bool isFocused();

private:
    RoundedRect rect;
    sf::Text text;
    sf::Text hint;
    string buffer;
    string placeholderText;
    float posX, posY, sizeW, sizeH;
    bool focused;
    float caretBlink;
};
