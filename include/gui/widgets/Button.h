#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "gui/widgets/RoundedRect.h"

using namespace std;

// A rounded button with hover and press feedback. There are no callbacks:
// the owning screen calls handleEvent and reacts when it returns true.
class Button
{
public:
    Button();

    void init(sf::Font* font, const string& text, float x, float y, float width, float height);
    void setLabel(const string& text);
    void setEnabled(bool on);
    void setToggled(bool on);               // stays highlighted (used for tabs)
    void setAccent(bool on);                // gold styling for the primary action
    bool handleEvent(const sf::Event& e);   // true exactly when clicked
    void update(float dt, float mouseX, float mouseY);
    void draw(sf::RenderWindow& window);
    bool contains(float px, float py);

    string label;

private:
    RoundedRect rect;
    sf::Text labelText;
    float posX, posY, sizeW, sizeH;
    bool enabled;
    bool toggled;
    bool accent;
    bool hovered;
    bool pressed;
    float glow;        // 0..1 hover animation

    void refreshStyle();
};
