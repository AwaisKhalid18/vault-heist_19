#pragma once

#include <SFML/Graphics.hpp>

// Gold particles that burst outward on a win.
class ParticleBurst
{
public:
    ParticleBurst();

    void burst(float x, float y, int count, int seed);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    bool isActive();

private:
    static const int MAX_PARTICLES = 220;
    float px[MAX_PARTICLES];
    float py[MAX_PARTICLES];
    float vx[MAX_PARTICLES];
    float vy[MAX_PARTICLES];
    float life[MAX_PARTICLES];
    int liveCount;
    sf::VertexArray quads;
};
