#include "gui/Background.h"
#include "gui/Theme.h"
#include <cmath>

static const float BG_PI = 3.14159265f;

// A tiny deterministic generator, so the field looks the same every run
// without pulling in <random> or touching the game's Random class.
static unsigned int bgSeed = 2024u;
static float bgRandom01()
{
    bgSeed = bgSeed * 1103515245u + 12345u;
    return (float)((bgSeed >> 16) & 0x7FFF) / 32767.f;
}

Background::Background()
{
    builtWidth = 0.f;
    builtHeight = 0.f;
    elapsed = 0.f;
    grid.setPrimitiveType(sf::Lines);
    vignette.setPrimitiveType(sf::TriangleFan);
    glow.setPrimitiveType(sf::Triangles);
    torches.setPrimitiveType(sf::Triangles);
    mist.setPrimitiveType(sf::Triangles);
    ash.setPrimitiveType(sf::Quads);
    embers.setPrimitiveType(sf::Quads);

    for (int i = 0; i < EMBER_COUNT; i++)
    {
        emberX[i] = 0.f; emberY[i] = 0.f; emberRise[i] = 0.f;
        emberSize[i] = 0.f; emberPhase[i] = 0.f; emberSway[i] = 0.f; emberHeat[i] = 0.f;
    }
    for (int i = 0; i < ASH_COUNT; i++)
    {
        ashX[i] = 0.f; ashY[i] = 0.f; ashFall[i] = 0.f; ashSize[i] = 0.f; ashPhase[i] = 0.f;
    }
    for (int i = 0; i < MIST_COUNT; i++)
    {
        mistX[i] = 0.f; mistY[i] = 0.f; mistSpeed[i] = 0.f;
        mistW[i] = 0.f; mistH[i] = 0.f; mistAlpha[i] = 0.f;
    }
}

// Layered sines standing in for noise: no clean beat for the eye to lock onto.
float Background::flicker(float seed, float speed)
{
    float a = sinf(elapsed * speed + seed);
    float b = sinf(elapsed * speed * 2.37f + seed * 1.7f);
    float c = sinf(elapsed * speed * 5.11f + seed * 0.6f);
    return 0.62f + 0.22f * a + 0.11f * b + 0.05f * c;
}

// A soft radial blob: bright core fading to nothing at the rim.
void Background::addBlob(sf::VertexArray& out, float cx, float cy, float rx, float ry,
                         const sf::Color& core, int segments)
{
    sf::Color edge = Theme::withAlpha(core, 0);
    sf::Vector2f centre(cx, cy);
    for (int i = 0; i < segments; i++)
    {
        float a0 = 2.f * BG_PI * (float)i / (float)segments;
        float a1 = 2.f * BG_PI * (float)(i + 1) / (float)segments;
        out.append(sf::Vertex(centre, core));
        out.append(sf::Vertex(sf::Vector2f(cx + rx * cosf(a0), cy + ry * sinf(a0)), edge));
        out.append(sf::Vertex(sf::Vector2f(cx + rx * cosf(a1), cy + ry * sinf(a1)), edge));
    }
}

void Background::seedField(float width, float height)
{
    bgSeed = 2024u;

    for (int i = 0; i < EMBER_COUNT; i++)
    {
        emberX[i] = bgRandom01() * width;
        emberY[i] = bgRandom01() * height;
        emberRise[i] = 9.f + bgRandom01() * 26.f;
        emberSize[i] = 1.f + bgRandom01() * 2.4f;
        emberPhase[i] = bgRandom01() * 2.f * BG_PI;
        emberSway[i] = 6.f + bgRandom01() * 18.f;
        emberHeat[i] = bgRandom01();
    }
    for (int i = 0; i < ASH_COUNT; i++)
    {
        ashX[i] = bgRandom01() * width;
        ashY[i] = bgRandom01() * height;
        ashFall[i] = 5.f + bgRandom01() * 13.f;
        ashSize[i] = 0.7f + bgRandom01() * 1.3f;
        ashPhase[i] = bgRandom01() * 2.f * BG_PI;
    }
    for (int i = 0; i < MIST_COUNT; i++)
    {
        mistY[i] = (0.1f + 0.85f * ((float)i / (float)MIST_COUNT)) * height;
        mistX[i] = bgRandom01() * width;
        mistSpeed[i] = 5.f + bgRandom01() * 13.f;
        if ((i % 2) == 1) { mistSpeed[i] = -mistSpeed[i]; }   // alternate directions
        mistW[i] = width * (0.38f + bgRandom01() * 0.42f);
        mistH[i] = 46.f + bgRandom01() * 92.f;
        mistAlpha[i] = 11.f + bgRandom01() * 13.f;
    }
}

void Background::stepEmbers(float dt, float width, float height)
{
    for (int i = 0; i < EMBER_COUNT; i++)
    {
        emberY[i] -= emberRise[i] * dt;
        if (emberY[i] < -8.f)
        {
            emberY[i] = height + 8.f;
            emberX[i] = bgRandom01() * width;
            emberHeat[i] = bgRandom01();
        }
    }

    embers.clear();
    for (int i = 0; i < EMBER_COUNT; i++)
    {
        float sway = sinf(elapsed * 0.55f + emberPhase[i]) * emberSway[i];
        float x = emberX[i] + sway;
        float y = emberY[i];

        float life = y / (height > 1.f ? height : 1.f);
        if (life < 0.f) { life = 0.f; }
        if (life > 1.f) { life = 1.f; }
        float edge = life < 0.18f ? life / 0.18f : (life > 0.88f ? (1.f - life) / 0.12f : 1.f);
        float fl = 0.55f + 0.45f * sinf(elapsed * 2.1f + emberPhase[i] * 1.7f);

        // embers cool as they climb: gold low down, deep orange near the top
        sf::Color warm = Theme::mix(Theme::emberDeep(), Theme::ember(),
                                    emberHeat[i] * life);
        int alpha = (int)(150.f * edge * fl);
        if (alpha < 0) { alpha = 0; }
        if (alpha > 255) { alpha = 255; }
        sf::Color c = Theme::withAlpha(warm, alpha);

        float s = emberSize[i] * (0.85f + 0.3f * fl);
        embers.append(sf::Vertex(sf::Vector2f(x - s, y - s), c));
        embers.append(sf::Vertex(sf::Vector2f(x + s, y - s), c));
        embers.append(sf::Vertex(sf::Vector2f(x + s, y + s), c));
        embers.append(sf::Vertex(sf::Vector2f(x - s, y + s), c));
    }
}

void Background::stepAsh(float dt, float width, float height)
{
    for (int i = 0; i < ASH_COUNT; i++)
    {
        ashY[i] += ashFall[i] * dt;
        if (ashY[i] > height + 6.f)
        {
            ashY[i] = -6.f;
            ashX[i] = bgRandom01() * width;
        }
    }

    ash.clear();
    sf::Color pale = Theme::ash();
    for (int i = 0; i < ASH_COUNT; i++)
    {
        float sway = sinf(elapsed * 0.33f + ashPhase[i]) * 16.f;
        float x = ashX[i] + sway;
        float y = ashY[i];
        float tw = 0.5f + 0.5f * sinf(elapsed * 0.9f + ashPhase[i] * 2.1f);
        int alpha = (int)(30.f + 26.f * tw);
        sf::Color c = Theme::withAlpha(pale, alpha);
        float s = ashSize[i];
        ash.append(sf::Vertex(sf::Vector2f(x - s, y - s), c));
        ash.append(sf::Vertex(sf::Vector2f(x + s, y - s), c));
        ash.append(sf::Vertex(sf::Vector2f(x + s, y + s), c));
        ash.append(sf::Vertex(sf::Vector2f(x - s, y + s), c));
    }
}

void Background::stepMist(float dt, float width, float height)
{
    mist.clear();
    sf::Color cold = Theme::mist();
    for (int i = 0; i < MIST_COUNT; i++)
    {
        mistX[i] += mistSpeed[i] * dt;
        float span = width + mistW[i] * 2.f;
        if (mistX[i] > width + mistW[i]) { mistX[i] -= span; }
        if (mistX[i] < -mistW[i]) { mistX[i] += span; }

        float breathe = 0.75f + 0.25f * sinf(elapsed * 0.21f + (float)i * 1.3f);
        int alpha = (int)(mistAlpha[i] * breathe);
        float drift = sinf(elapsed * 0.17f + (float)i) * 10.f;
        addBlob(mist, mistX[i], mistY[i] + drift, mistW[i] * 0.5f, mistH[i] * 0.5f,
                Theme::withAlpha(cold, alpha), 26);
    }
}

void Background::buildGlow(float width, float height, float strength)
{
    glow.clear();
    float cx = width / 2.f;
    float cy = height * 0.46f;
    float radius = (width < height ? width : height) * 0.66f;

    int centreAlpha = (int)(40.f * strength);
    if (centreAlpha < 0) { centreAlpha = 0; }
    if (centreAlpha > 255) { centreAlpha = 255; }
    addBlob(glow, cx, cy, radius, radius * 0.86f,
            Theme::withAlpha(Theme::hearthGlow(), centreAlpha), 40);
}

void Background::buildTorches(float width, float height)
{
    torches.clear();
    float ty = height * 0.34f;
    float rx = width * 0.17f;
    float ry = height * 0.30f;

    float leftF = flicker(1.7f, 2.3f);
    float rightF = flicker(5.2f, 2.0f);

    int la = (int)(54.f * leftF);
    int ra = (int)(54.f * rightF);
    if (la < 0) { la = 0; } if (la > 255) { la = 255; }
    if (ra < 0) { ra = 0; } if (ra > 255) { ra = 255; }

    addBlob(torches, width * 0.045f, ty, rx, ry,
            Theme::withAlpha(Theme::torchGlow(), la), 30);
    addBlob(torches, width * 0.955f, ty, rx, ry,
            Theme::withAlpha(Theme::torchGlow(), ra), 30);
}

void Background::build(float width, float height)
{
    if (width == builtWidth && height == builtHeight)
    {
        return;
    }
    builtWidth = width;
    builtHeight = height;

    // stonework lines, one every 48 pixels
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

    // vignette: transparent centre fading to a deep dark rim
    vignette.clear();
    const int SEGMENTS = 56;
    float cx = width / 2.f;
    float cy = height / 2.f;
    float radius = sqrtf(cx * cx + cy * cy) * 1.05f;
    vignette.append(sf::Vertex(sf::Vector2f(cx, cy), sf::Color(0, 0, 0, 0)));
    for (int i = 0; i <= SEGMENTS; i++)
    {
        float angle = 2.f * BG_PI * (float)i / (float)SEGMENTS;
        sf::Vector2f p(cx + radius * cosf(angle), cy + radius * sinf(angle));
        vignette.append(sf::Vertex(p, sf::Color(0, 0, 0, 186)));
    }

    seedField(width, height);
}

void Background::draw(sf::RenderWindow& window)
{
    sf::Vector2u size = window.getSize();
    float w = (float)size.x;
    float h = (float)size.y;
    build(w, h);

    // Clamped: a screen that has not drawn for a while (or a debugger pause)
    // must not teleport the whole field in one frame.
    float dt = clock.restart().asSeconds();
    if (dt < 0.f) { dt = 0.f; }
    if (dt > 0.05f) { dt = 0.05f; }
    elapsed += dt;

    float strength = 0.80f + 0.20f * sinf(elapsed * 0.7f)
                           + 0.07f * sinf(elapsed * 1.9f + 1.3f);
    buildGlow(w, h, strength);
    buildTorches(w, h);
    stepMist(dt, w, h);
    stepAsh(dt, w, h);
    stepEmbers(dt, w, h);

    window.draw(grid);
    window.draw(glow, sf::BlendAdd);
    window.draw(torches, sf::BlendAdd);
    window.draw(mist, sf::BlendAlpha);
    window.draw(ash, sf::BlendAlpha);
    window.draw(embers, sf::BlendAdd);
    window.draw(vignette);
}
