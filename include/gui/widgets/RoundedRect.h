#pragma once

#include <SFML/Graphics.hpp>

// A rectangle with rounded corners built from an sf::ConvexShape
// (SFML has no rounded rectangle of its own). Used for every panel.
class RoundedRect
{
public:
    RoundedRect();

    void setRect(float x, float y, float width, float height, float radius);
    void setFillColor(const sf::Color& color);
    void setOutline(const sf::Color& color, float thickness);
    void draw(sf::RenderWindow& window);
    void drawWithShadow(sf::RenderWindow& window);
    bool contains(float px, float py);

    float left();
    float top();
    float width();
    float height();

private:
    sf::ConvexShape shape;
    sf::ConvexShape shadow;
    float rectX, rectY, rectW, rectH;

    void buildShape(sf::ConvexShape& target, float x, float y, float w, float h, float radius);
};
