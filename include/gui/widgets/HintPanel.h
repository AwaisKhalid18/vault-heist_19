#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "gui/widgets/RoundedRect.h"

using namespace std;

// A hint card that slides in from the right edge and slides away when
// dismissed (H again, or its own timer).
class HintPanel
{
public:
    HintPanel();

    void init(sf::Font* headingFont, sf::Font* textFont);
    void show(const string& hintText);
    void hide();
    bool isVisible();
    void update(float dt, float windowW);
    void draw(sf::RenderWindow& window);

private:
    RoundedRect panel;
    sf::Text title;
    sf::Text body;
    string text;
    float slide;          // 0 hidden .. 1 fully shown
    bool visible;
    float windowWidth;
};
