#include "gui/widgets/RoundedRect.h"
#include <cmath>

RoundedRect::RoundedRect()
{
    rectX = 0.f;
    rectY = 0.f;
    rectW = 0.f;
    rectH = 0.f;
    shadow.setFillColor(sf::Color(0, 0, 0, 90));
}

// 8 points per corner trace a quarter circle, 32 points in total.
void RoundedRect::buildShape(sf::ConvexShape& target, float x, float y, float w, float h, float radius)
{
    const int CORNER_POINTS = 8;
    const float PI = 3.14159265f;
    if (radius > w / 2.f)
    {
        radius = w / 2.f;
    }
    if (radius > h / 2.f)
    {
        radius = h / 2.f;
    }
    target.setPointCount(CORNER_POINTS * 4);

    // corner centres, clockwise from top-left
    float cx[4] = { x + radius, x + w - radius, x + w - radius, x + radius };
    float cy[4] = { y + radius, y + radius, y + h - radius, y + h - radius };
    float startAngle[4] = { PI, 1.5f * PI, 0.f, 0.5f * PI };

    int point = 0;
    for (int corner = 0; corner < 4; corner++)
    {
        for (int i = 0; i < CORNER_POINTS; i++)
        {
            float angle = startAngle[corner] + (0.5f * PI) * ((float)i / (float)(CORNER_POINTS - 1));
            float px = cx[corner] + radius * cosf(angle);
            float py = cy[corner] + radius * sinf(angle);
            target.setPoint(point, sf::Vector2f(px, py));
            point++;
        }
    }
}

void RoundedRect::setRect(float x, float y, float width, float height, float radius)
{
    rectX = x;
    rectY = y;
    rectW = width;
    rectH = height;
    buildShape(shape, x, y, width, height, radius);
    buildShape(shadow, x + 3.f, y + 5.f, width, height, radius);
}

void RoundedRect::setFillColor(const sf::Color& color)
{
    shape.setFillColor(color);
}

void RoundedRect::setOutline(const sf::Color& color, float thickness)
{
    shape.setOutlineColor(color);
    shape.setOutlineThickness(thickness);
}

void RoundedRect::draw(sf::RenderWindow& window)
{
    window.draw(shape);
}

void RoundedRect::drawWithShadow(sf::RenderWindow& window)
{
    window.draw(shadow);
    window.draw(shape);
}

bool RoundedRect::contains(float px, float py)
{
    return px >= rectX && px <= rectX + rectW && py >= rectY && py <= rectY + rectH;
}

float RoundedRect::left() { return rectX; }
float RoundedRect::top() { return rectY; }
float RoundedRect::width() { return rectW; }
float RoundedRect::height() { return rectH; }
