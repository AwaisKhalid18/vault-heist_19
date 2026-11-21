#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "gui/widgets/RoundedRect.h"
#include "gui/widgets/Button.h"
#include "gui/widgets/KnightHerald.h"

using namespace std;

// A modal dialog: dimmed screen, centred panel, title, wrapped body text
// and up to five buttons. handleEvent returns the clicked button's index.
class Dialog
{
public:
    static const int MAX_DIALOG_BUTTONS = 5;

    Dialog();

    void init(sf::Font* titleFont, sf::Font* textFont);
    void open(const string& titleText, const string& bodyText,
              const string buttonLabels[], int buttonCount);
    void open2(const string& titleText, const string& bodyText,
               const string& button1, const string& button2);   // common case
    void close();
    bool isOpen();
    int handleEvent(const sf::Event& e);     // button index or -1
    void update(float dt, float mouseX, float mouseY);
    void draw(sf::RenderWindow& window);

private:
    RoundedRect panel;
    sf::Text title;
    sf::Text body;
    KnightHerald herald;
    float heraldX, heraldY, heraldSide;
    Button buttons[MAX_DIALOG_BUTTONS];
    int buttonCount;
    bool openFlag;
    float fade;            // 0..1 open animation
    string pendingTitle;
    string pendingBody;
    string pendingLabels[MAX_DIALOG_BUTTONS];
    sf::Font* titleFontPtr;
    sf::Font* textFontPtr;

    void layout(float windowW, float windowH);
    string wrap(const string& text, int maxChars);
};
