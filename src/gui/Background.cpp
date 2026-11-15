#include "gui/Background.h"
#include "gui/Theme.h"
#include <cmath>

Background::Background()
{
    builtWidth = 0.f;
    builtHeight = 0.f;
    grid.setPrimitiveType(sf::Lines);
    vignette.setPrimitiveType(sf::TriangleFan);
}

void Background::build(float width, float height)
{
    if (width == builtWidth && height == builtHeight)
    {
        return;
    }
    builtWidth = width;
    builtHeight = height;

    // blueprint grid, one line every 48 pixels
    grid.clear();
    const float STEP = 48.f;
    sf::Color line = Theme::gridLine();
    for (float x = 0.f; x <= width; x += STEP)
    {
        grid.append(sf::Vertex(sf::Vector2f(x, 0.f), line));
        grid.append(sf::Vertex(sf::Vector2f(x, height), line));
    }
    for (float y = 0.f; y <= height; y += STEP)
    {
        grid.append(sf::Vertex(sf::Vector2f(0.f, y), line));
        grid.append(sf::Vertex(sf::Vector2f(width, y), line));
    }

    // vignette: transparent centre fading to dark edges, as a triangle fan
    vignette.clear();
    const int SEGMENTS = 48;
    const float PI = 3.14159265f;
    float cx = width / 2.f;
    float cy = height / 2.f;
    float radius = sqrtf(cx * cx + cy * cy) * 1.05f;
    vignette.append(sf::Vertex(sf::Vector2f(cx, cy), sf::Color(0, 0, 0, 0)));
    for (int i = 0; i <= SEGMENTS; i++)
    {
        float angle = 2.f * PI * (float)i / (float)SEGMENTS;
        sf::Vector2f p(cx + radius * cosf(angle), cy + radius * sinf(angle));
        vignette.append(sf::Vertex(p, sf::Color(0, 0, 0, 110)));
    }
}

void Background::draw(sf::RenderWindow& window)
{
    sf::Vector2u size = window.getSize();
    build((float)size.x, (float)size.y);
    window.draw(grid);
    window.draw(vignette);
}
