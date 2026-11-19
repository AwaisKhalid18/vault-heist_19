#pragma once

#include <SFML/Graphics.hpp>
#include <string>

using namespace std;

// A horizontal slider with a draggable knob. Stores a value in [0, 1];
// the owner maps that to whatever range it needs (speed, tree size...).
class Slider
{
public:
    Slider();

    void init(float x, float y, float width);
    void setValue(float v01);
    float getValue();
    bool handleEvent(const sf::Event& e);   // true while the value changes
    void draw(sf::RenderWindow& window);

private:
    float posX, posY, sizeW;
    float value;
    bool dragging;

    void setFromMouse(float mouseX);
};
