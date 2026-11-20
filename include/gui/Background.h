#pragma once

#include <SFML/Graphics.hpp>

// The blueprint backdrop: a faint grid plus a soft dark vignette so the
// screen centre glows a little. Drawn first on every screen.
class Background
{
public:
    Background();

    void build(float width, float height);
    void draw(sf::RenderWindow& window);

private:
    sf::VertexArray grid;
    sf::VertexArray vignette;
    float builtWidth;
    float builtHeight;
};
