#include "gui/ParticleBurst.h"
#include "gui/Theme.h"
#include "core/Random.h"
#include <cmath>

ParticleBurst::ParticleBurst()
{
    liveCount = 0;
    quads.setPrimitiveType(sf::Quads);
}

void ParticleBurst::burst(float x, float y, int count, int seed)
{
    const float PI = 3.14159265f;
    Random rng(seed);
    if (count > MAX_PARTICLES)
    {
        count = MAX_PARTICLES;
    }
    liveCount = count;
    for (int i = 0; i < count; i++)
    {
        float angle = 2.f * PI * (float)rng.nextInt(0, 999) / 1000.f;
        float speed = 120.f + (float)rng.nextInt(0, 260);
        px[i] = x;
        py[i] = y;
        vx[i] = cosf(angle) * speed;
        vy[i] = sinf(angle) * speed - 80.f;
        life[i] = 0.8f + (float)rng.nextInt(0, 100) / 120.f;
    }
}

void ParticleBurst::update(float dt)
{
    for (int i = 0; i < liveCount; i++)
    {
        life[i] -= dt;
        if (life[i] <= 0.f)
        {
            // replace the dead particle with the last live one
            liveCount--;
            px[i] = px[liveCount];
            py[i] = py[liveCount];
            vx[i] = vx[liveCount];
            vy[i] = vy[liveCount];
            life[i] = life[liveCount];
            i--;
            continue;
        }
        vy[i] += 360.f * dt;    // gravity
        px[i] += vx[i] * dt;
        py[i] += vy[i] * dt;
    }
}

void ParticleBurst::draw(sf::RenderWindow& window)
{
    if (liveCount == 0)
    {
        return;
    }
    quads.clear();
    for (int i = 0; i < liveCount; i++)
    {
        float s = 2.f + life[i] * 2.f;
        int alpha = (int)(life[i] * 255.f);
        if (alpha > 255)
        {
            alpha = 255;
        }
        sf::Color c = Theme::withAlpha(Theme::gold(), alpha);
        quads.append(sf::Vertex(sf::Vector2f(px[i] - s, py[i] - s), c));
        quads.append(sf::Vertex(sf::Vector2f(px[i] + s, py[i] - s), c));
        quads.append(sf::Vertex(sf::Vector2f(px[i] + s, py[i] + s), c));
        quads.append(sf::Vertex(sf::Vector2f(px[i] - s, py[i] + s), c));
    }
    window.draw(quads);
}

bool ParticleBurst::isActive()
{
    return liveCount > 0;
}
