#include "gui/widgets/Tooltip.h"
#include "gui/Theme.h"

Tooltip::Tooltip()
{
    visible = false;
    posX = 0.f;
    posY = 0.f;
}

void Tooltip::init(sf::Font* font)
{
    text.setFont(*font);
    text.setCharacterSize(Theme::smallSize());
    text.setFillColor(Theme::text());
}

void Tooltip::show(const string& content, float mouseX, float mouseY)
{
    text.setString(content);
    posX = mouseX + 18.f;
    posY = mouseY + 14.f;
    visible = true;
}

void Tooltip::hide()
{
    visible = false;
}

void Tooltip::draw(sf::RenderWindow& window)
{
    if (!visible)
    {
        return;
    }
    sf::FloatRect bounds = text.getLocalBounds();
    float w = bounds.width + 24.f;
    float h = bounds.height + 20.f;

    // keep the box inside the window
    sf::Vector2u size = window.getSize();
    float x = posX;
    float y = posY;
    if (x + w > (float)size.x)
    {
        x = (float)size.x - w - 6.f;
    }
    if (y + h > (float)size.y)
    {
        y = posY - h - 24.f;
    }

    box.setRect(x, y, w, h, 8.f);
    box.setFillColor(sf::Color(8, 11, 18, 245));
    box.setOutline(Theme::currentVisit(), 1.f);
    box.draw(window);
    text.setPosition(x + 12.f - bounds.left, y + 10.f - bounds.top);
    window.draw(text);
}
