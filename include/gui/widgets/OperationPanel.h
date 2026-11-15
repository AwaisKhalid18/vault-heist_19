#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "gui/widgets/RoundedRect.h"
#include "gui/widgets/Button.h"
#include "gui/widgets/TextField.h"

using namespace std;

// A panel holding a grid of operation buttons and up to two number boxes.
// The owner adds buttons by label and reacts to the returned index.
class OperationPanel
{
public:
    static const int MAX_BUTTONS = 24;

    OperationPanel();

    void init(sf::Font* headingFont, sf::Font* textFont, const string& titleText);
    void setRect(float x, float y, float width, float height);
    void clearButtons();
    int addButton(const string& label);            // returns the button's index
    void setFieldCount(int count);                 // 0, 1 or 2 input boxes
    void setFieldPlaceholders(const string& a, const string& b);
    void relayout();                               // call after adding buttons

    int handleEvent(const sf::Event& e);           // clicked button index or -1
    void update(float dt, float mouseX, float mouseY);
    void draw(sf::RenderWindow& window);

    bool getFieldInt(int field, int& out);
    void clearFields();
    void setButtonEnabled(int index, bool on);
    void setButtonToggled(int index, bool on);
    bool anyFieldFocused();

private:
    RoundedRect panel;
    sf::Text title;
    Button buttons[MAX_BUTTONS];
    string labels[MAX_BUTTONS];
    int buttonCount;
    TextField fieldA;
    TextField fieldB;
    int fieldCount;
    string placeholderA, placeholderB;
    float posX, posY, sizeW, sizeH;
    sf::Font* textFont;
};
