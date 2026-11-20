#pragma once

#include <SFML/Graphics.hpp>
#include "core/EventLog.h"
#include "gui/widgets/RoundedRect.h"

// Scrolling list of the current operation's events in a monospace font.
// The event being animated right now is highlighted; the wheel scrolls
// when the mouse is over the panel, otherwise it follows the animation.
class EventLogPanel
{
public:
    EventLogPanel();

    void init(sf::Font* headingFont, sf::Font* monoFont);
    void setRect(float x, float y, float width, float height);
    bool handleEvent(const sf::Event& e);
    void draw(sf::RenderWindow& window, EventLog* log, int currentIndex);

private:
    RoundedRect panel;
    sf::Text title;
    sf::Text line;
    float posX, posY, sizeW, sizeH;
    int scrollOffset;          // first visible event index
    bool userScrolled;
};
