#include "gui/widgets/Button.h"
#include "gui/Theme.h"

Button::Button()
{
    posX = 0.f;
    posY = 0.f;
    sizeW = 0.f;
    sizeH = 0.f;
    enabled = true;
    toggled = false;
    accent = false;
    hovered = false;
    pressed = false;
    glow = 0.f;
    label = "";
}

void Button::init(sf::Font* font, const string& text, float x, float y, float width, float height)
{
    posX = x;
    posY = y;
    sizeW = width;
    sizeH = height;
    label = text;
    labelText.setFont(*font);
    labelText.setCharacterSize(Theme::textSize());
    labelText.setString(text);
    rect.setRect(x, y, width, height, 9.f);
    refreshStyle();
}

void Button::setLabel(const string& text)
{
    label = text;
    labelText.setString(text);
}

void Button::setEnabled(bool on)
{
    enabled = on;
    refreshStyle();
}

void Button::setToggled(bool on)
{
    toggled = on;
    refreshStyle();
}

void Button::setAccent(bool on)
{
    accent = on;
    refreshStyle();
}

bool Button::contains(float px, float py)
{
    return rect.contains(px, py);
}

bool Button::handleEvent(const sf::Event& e)
{
    if (!enabled)
    {
        return false;
    }
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
    {
        if (contains((float)e.mouseButton.x, (float)e.mouseButton.y))
        {
            pressed = true;
        }
    }
    if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left)
    {
        bool wasPressed = pressed;
        pressed = false;
        if (wasPressed && contains((float)e.mouseButton.x, (float)e.mouseButton.y))
        {
            return true;
        }
    }
    return false;
}

void Button::update(float dt, float mouseX, float mouseY)
{
    hovered = enabled && contains(mouseX, mouseY);
    float target = hovered ? 1.f : 0.f;
    float speed = 8.f * dt;
    if (glow < target)
    {
        glow = glow + speed < target ? glow + speed : target;
    }
    else
    {
        glow = glow - speed > target ? glow - speed : target;
    }
    refreshStyle();
}

void Button::refreshStyle()
{
    sf::Color base = toggled ? Theme::panelLight() : Theme::panel();
    sf::Color hover = Theme::panelLight();
    sf::Color fill = Theme::mix(base, hover, glow);
    sf::Color border = toggled ? Theme::currentVisit() : Theme::panelBorder();
    sf::Color textColor = enabled ? Theme::text() : Theme::dimText();
    if (accent)
    {
        fill = Theme::mix(sf::Color(64, 50, 20), sf::Color(96, 74, 26), glow);
        border = Theme::gold();
        textColor = enabled ? sf::Color(250, 230, 180) : Theme::dimText();
    }
    if (pressed)
    {
        fill = Theme::mix(fill, sf::Color::Black, 0.25f);
    }
    rect.setFillColor(fill);
    rect.setOutline(Theme::mix(border, Theme::currentVisit(), accent ? 0.f : glow * 0.6f), 1.f);
    labelText.setFillColor(textColor);
}

void Button::draw(sf::RenderWindow& window)
{
    rect.draw(window);
    sf::FloatRect bounds = labelText.getLocalBounds();
    labelText.setPosition(posX + (sizeW - bounds.width) / 2.f - bounds.left,
                          posY + (sizeH - bounds.height) / 2.f - bounds.top - 1.f);
    window.draw(labelText);
}
