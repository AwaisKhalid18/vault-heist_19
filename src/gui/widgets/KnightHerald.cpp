#include "gui/widgets/KnightHerald.h"
#include "gui/Theme.h"
#include <cmath>

// The bust is painted once into a square texture and then drawn onto a
// circle, which is what trims it neatly to the medallion.
static const unsigned int BUST_SIZE = 256;

static void ellipse(sf::RenderTarget& target, float cx, float cy,
                    float rx, float ry, const sf::Color& color)
{
    sf::CircleShape shape(1.f, 36);
    shape.setOrigin(1.f, 1.f);
    shape.setScale(rx, ry);
    shape.setPosition(cx, cy);
    shape.setFillColor(color);
    target.draw(shape);
}

// A thick line, used for the blade and its fuller.
static void bar(sf::RenderTarget& target, float x1, float y1, float x2, float y2,
                float thickness, const sf::Color& color)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.001f)
    {
        return;
    }
    sf::RectangleShape r(sf::Vector2f(len, thickness));
    r.setOrigin(0.f, thickness / 2.f);
    r.setPosition(x1, y1);
    r.setRotation(atan2f(dy, dx) * 180.f / 3.14159265f);
    r.setFillColor(color);
    target.draw(r);
}

KnightHerald::KnightHerald()
{
    triedLoad = false;
    hasFile = false;
    bustReady = false;
    elapsed = 0.f;
}

void KnightHerald::ensureLoaded()
{
    if (triedLoad)
    {
        return;
    }
    triedLoad = true;
    hasFile = fileTexture.loadFromFile("assets/ui/herald.png")
           || fileTexture.loadFromFile("../assets/ui/herald.png");
    if (hasFile)
    {
        fileTexture.setSmooth(true);
    }
}

void KnightHerald::paintBust()
{
    if (bustReady)
    {
        return;
    }
    if (!bust.create(BUST_SIZE, BUST_SIZE))
    {
        return;
    }

    const sf::Color STONE(26, 25, 30);
    const sf::Color CLOAK(104, 26, 30);
    const sf::Color CLOAK_DARK(72, 18, 22);
    const sf::Color STEEL(118, 124, 136);
    const sf::Color STEEL_DARK(82, 88, 99);
    const sf::Color STEEL_LIT(168, 176, 190);
    const sf::Color TRIM = Theme::gold();
    const sf::Color SKIN(222, 186, 156);
    const sf::Color SKIN_SHADE(196, 160, 132);
    const sf::Color HAIR(190, 152, 84);
    const sf::Color HAIR_LIT(222, 188, 120);
    const sf::Color DARK(32, 26, 20);

    bust.clear(STONE);

    // a warm pool behind the shoulders, so the figure sits in torchlight
    ellipse(bust, 128.f, 150.f, 120.f, 120.f, sf::Color(58, 40, 24));
    ellipse(bust, 128.f, 140.f, 86.f, 86.f, sf::Color(74, 50, 28));

    // cloak, then the armoured shoulders over it
    ellipse(bust, 128.f, 244.f, 118.f, 74.f, CLOAK_DARK);
    ellipse(bust, 128.f, 250.f, 102.f, 64.f, CLOAK);
    ellipse(bust, 128.f, 258.f, 86.f, 56.f, STEEL_DARK);
    ellipse(bust, 128.f, 262.f, 78.f, 50.f, STEEL);

    // pauldrons
    ellipse(bust, 58.f, 236.f, 34.f, 27.f, STEEL);
    ellipse(bust, 198.f, 236.f, 34.f, 27.f, STEEL);
    ellipse(bust, 58.f, 231.f, 30.f, 22.f, STEEL_LIT);
    ellipse(bust, 198.f, 231.f, 30.f, 22.f, STEEL_LIT);
    ellipse(bust, 58.f, 229.f, 24.f, 16.f, STEEL);
    ellipse(bust, 198.f, 229.f, 24.f, 16.f, STEEL);

    // gorget with a gold edge
    ellipse(bust, 128.f, 214.f, 46.f, 24.f, TRIM);
    ellipse(bust, 128.f, 218.f, 41.f, 20.f, STEEL_DARK);

    // neck and jaw
    sf::RectangleShape neck(sf::Vector2f(42.f, 46.f));
    neck.setPosition(107.f, 160.f);
    neck.setFillColor(SKIN_SHADE);
    bust.draw(neck);

    ellipse(bust, 128.f, 118.f, 44.f, 52.f, SKIN);
    ellipse(bust, 128.f, 140.f, 38.f, 36.f, SKIN);          // jaw
    ellipse(bust, 101.f, 120.f, 7.f, 11.f, SKIN_SHADE);     // ears
    ellipse(bust, 155.f, 120.f, 7.f, 11.f, SKIN_SHADE);

    // hair: swept back, blond, with a lit edge
    ellipse(bust, 128.f, 86.f, 50.f, 38.f, HAIR);
    ellipse(bust, 128.f, 78.f, 46.f, 30.f, HAIR_LIT);
    ellipse(bust, 92.f, 112.f, 13.f, 32.f, HAIR);
    ellipse(bust, 164.f, 112.f, 13.f, 32.f, HAIR);
    ellipse(bust, 128.f, 94.f, 44.f, 20.f, HAIR);           // fringe

    // brows, eyes, mouth: a few marks, kept simple so it reads at small size
    bar(bust, 108.f, 112.f, 124.f, 109.f, 4.f, DARK);
    bar(bust, 132.f, 109.f, 148.f, 112.f, 4.f, DARK);
    ellipse(bust, 115.f, 121.f, 6.f, 4.f, sf::Color(248, 246, 240));
    ellipse(bust, 141.f, 121.f, 6.f, 4.f, sf::Color(248, 246, 240));
    ellipse(bust, 115.f, 121.f, 3.f, 3.f, sf::Color(74, 96, 92));
    ellipse(bust, 141.f, 121.f, 3.f, 3.f, sf::Color(74, 96, 92));
    bar(bust, 120.f, 152.f, 136.f, 152.f, 3.f, SKIN_SHADE);
    ellipse(bust, 128.f, 136.f, 5.f, 8.f, SKIN_SHADE);      // nose

    // the sword, carried across the shoulder
    bar(bust, 48.f, 250.f, 206.f, 58.f, 15.f, STEEL_DARK);
    bar(bust, 50.f, 248.f, 204.f, 60.f, 11.f, STEEL_LIT);
    bar(bust, 52.f, 246.f, 200.f, 64.f, 3.f, sf::Color(228, 234, 244));
    bar(bust, 40.f, 262.f, 70.f, 226.f, 13.f, TRIM);        // grip
    bar(bust, 48.f, 268.f, 86.f, 228.f, 7.f, sf::Color(150, 112, 44));
    ellipse(bust, 72.f, 228.f, 13.f, 13.f, TRIM);           // pommel / guard

    bust.display();
    bustReady = true;
}

void KnightHerald::draw(sf::RenderWindow& window, float x, float y, float w, float h,
                        sf::Font* captionFont)
{
    ensureLoaded();
    paintBust();

    float dt = clock.restart().asSeconds();
    if (dt < 0.f) { dt = 0.f; }
    if (dt > 0.05f) { dt = 0.05f; }
    elapsed += dt;

    float captionRoom = captionFont != nullptr ? 20.f : 0.f;
    float side = w < (h - captionRoom) ? w : (h - captionRoom);
    if (side < 8.f)
    {
        return;
    }
    float radius = side / 2.f;
    float cx = x + w / 2.f;
    float cy = y + radius;

    // a soft halo so the medallion sits on the panel rather than on top of it
    float pulse = 0.72f + 0.28f * sinf(elapsed * 1.25f);
    sf::CircleShape halo(radius * 1.16f, 44);
    halo.setOrigin(radius * 1.16f, radius * 1.16f);
    halo.setPosition(cx, cy);
    halo.setFillColor(Theme::withAlpha(Theme::torchGlow(), (int)(34.f * pulse)));
    window.draw(halo, sf::BlendAdd);

    // the portrait itself, trimmed to the circle
    sf::CircleShape disc(radius, 56);
    disc.setOrigin(radius, radius);
    disc.setPosition(cx, cy);
    if (hasFile)
    {
        disc.setTexture(&fileTexture);
        sf::Vector2u ts = fileTexture.getSize();
        // centre crop, so a rectangular picture is not squashed
        unsigned int edge = ts.x < ts.y ? ts.x : ts.y;
        disc.setTextureRect(sf::IntRect((int)((ts.x - edge) / 2),
                                        (int)((ts.y - edge) / 2),
                                        (int)edge, (int)edge));
    }
    else if (bustReady)
    {
        disc.setTexture(&bust.getTexture());
        disc.setTextureRect(sf::IntRect(0, 0, (int)BUST_SIZE, (int)BUST_SIZE));
    }
    else
    {
        disc.setFillColor(Theme::panelLight());
    }
    window.draw(disc);

    // gilded rim, brightening gently with the halo
    sf::CircleShape rim(radius - 1.f, 56);
    rim.setOrigin(radius - 1.f, radius - 1.f);
    rim.setPosition(cx, cy);
    rim.setFillColor(sf::Color::Transparent);
    rim.setOutlineThickness(2.f);
    rim.setOutlineColor(Theme::mix(Theme::panelBorder(), Theme::gold(), 0.45f + 0.55f * pulse));
    window.draw(rim);

    if (captionFont != nullptr)
    {
        sf::Text caption;
        caption.setFont(*captionFont);
        caption.setCharacterSize(Theme::smallSize() - 1);
        caption.setString("THE HERALD");
        caption.setFillColor(Theme::dimText());
        sf::FloatRect b = caption.getLocalBounds();
        caption.setPosition(cx - b.width / 2.f - b.left, cy + radius + 5.f);
        window.draw(caption);
    }
}
