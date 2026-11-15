#include "gui/widgets/Slider.h"
#include "gui/Theme.h"

Slider::Slider()
{
    posX = 0.f;
    posY = 0.f;
    sizeW = 100.f;
    value = 0.5f;
    dragging = false;
}

void Slider::init(float x, float y, float width)
{
    posX = x;
    posY = y;
    sizeW = width;
}

void Slider::setValue(float v01)
{
    if (v01 < 0.f) { v01 = 0.f; }
    if (v01 > 1.f) { v01 = 1.f; }
    value = v01;
}

float Slider::getValue()
{
    return value;
}

void Slider::setFromMouse(float mouseX)
{
    setValue((mouseX - posX) / sizeW);
}

bool Slider::handleEvent(const sf::Event& e)
{
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
    {
        float mx = (float)e.mouseButton.x;
        float my = (float)e.mouseButton.y;
        if (mx >= posX - 8.f && mx <= posX + sizeW + 8.f && my >= posY - 12.f && my <= posY + 12.f)
        {
            dragging = true;
            setFromMouse(mx);
            return true;
        }
    }
    if (e.type == sf::Event::MouseButtonReleased)
    {
        dragging = false;
    }
    if (e.type == sf::Event::MouseMoved && dragging)
    {
        setFromMouse((float)e.mouseMove.x);
        return true;
    }
    return false;
}

void Slider::draw(sf::RenderWindow& window)
{
    // track
    sf::RectangleShape track(sf::Vector2f(sizeW, 4.f));
    track.setPosition(posX, posY - 2.f);
    track.setFillColor(Theme::panelBorder());
    window.draw(track);

    // filled part
    sf::RectangleShape fill(sf::Vector2f(sizeW * value, 4.f));
    fill.setPosition(posX, posY - 2.f);
    fill.setFillColor(Theme::currentVisit());
    window.draw(fill);

    // knob
    sf::CircleShape knob(8.f);
    knob.setOrigin(8.f, 8.f);
    knob.setPosition(posX + sizeW * value, posY);
    knob.setFillColor(Theme::text());
    knob.setOutlineColor(Theme::currentVisit());
    knob.setOutlineThickness(dragging ? 3.f : 1.f);
    window.draw(knob);
}
