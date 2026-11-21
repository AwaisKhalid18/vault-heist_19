#pragma once

#include <SFML/Graphics.hpp>

// The backdrop: a great hall at night. Layered back to front -
//   1. faint stonework lines
//   2. a breathing hearth glow behind the vault
//   3. two guttering wall torches
//   4. cold mist drifting across in slow bands
//   5. pale ash sifting down
//   6. embers climbing on the heat
//   7. a deep vignette closing the corners in
//
// It keeps its own clock and advances itself inside draw(), so no screen
// has to pass a delta time in. Purely decorative: nothing here is read by
// the game logic, and every layer sits behind the panels and the tree.
class Background
{
public:
    Background();

    void build(float width, float height);
    void draw(sf::RenderWindow& window);

private:
    static const int EMBER_COUNT = 96;
    static const int ASH_COUNT = 56;
    static const int MIST_COUNT = 6;

    sf::VertexArray grid;
    sf::VertexArray vignette;
    sf::VertexArray glow;
    sf::VertexArray torches;
    sf::VertexArray mist;
    sf::VertexArray ash;
    sf::VertexArray embers;
    float builtWidth;
    float builtHeight;

    sf::Clock clock;
    float elapsed;

    float emberX[EMBER_COUNT];
    float emberY[EMBER_COUNT];
    float emberRise[EMBER_COUNT];
    float emberSize[EMBER_COUNT];
    float emberPhase[EMBER_COUNT];
    float emberSway[EMBER_COUNT];
    float emberHeat[EMBER_COUNT];    // 0 = cooling and red, 1 = bright gold

    float ashX[ASH_COUNT];
    float ashY[ASH_COUNT];
    float ashFall[ASH_COUNT];
    float ashSize[ASH_COUNT];
    float ashPhase[ASH_COUNT];

    float mistY[MIST_COUNT];
    float mistX[MIST_COUNT];
    float mistSpeed[MIST_COUNT];
    float mistW[MIST_COUNT];
    float mistH[MIST_COUNT];
    float mistAlpha[MIST_COUNT];

    void seedField(float width, float height);
    void stepEmbers(float dt, float width, float height);
    void stepAsh(float dt, float width, float height);
    void stepMist(float dt, float width, float height);
    void buildGlow(float width, float height, float strength);
    void buildTorches(float width, float height);

    // soft radial blob appended to `out` as a triangle fan's worth of triangles
    void addBlob(sf::VertexArray& out, float cx, float cy, float rx, float ry,
                 const sf::Color& core, int segments);
    float flicker(float seed, float speed);
};
