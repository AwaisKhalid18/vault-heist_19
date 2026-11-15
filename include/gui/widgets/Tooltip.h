#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "gui/widgets/RoundedRect.h"

using namespace std;

// A small floating box next to the cursor showing a hovered room's
// key, height, balance factor, size and depth.
class Tooltip
{
public:
    Tooltip();

    void init(sf::Font* font);
    void show(const string& text, float mouseX, float mouseY);
    void hide();
    void draw(sf::RenderWindow& window);

private:
    sf::Text text;
    RoundedRect box;
    bool visible;
    float posX, posY;
};
