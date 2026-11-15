#pragma once

#include <SFML/Graphics.hpp>
#include "gui/AnimationController.h"

// Draws the vault: corridors (edges) in one vertex array, then every room
// as a metallic vault door with its key, badge and non-colour signals.
// Colour is never the only signal: violations get a "!", the target gets
// a double ring and a diamond, pruned rooms get a "pruned" label, and the
// current room gets a pulsing ring.
class TreeRenderer
{
public:
    // what the small badge next to each room shows
    static const int BADGE_NONE = 0, BADGE_HEIGHT = 1, BADGE_BF = 2, BADGE_SIZE = 3;

    TreeRenderer();

    void init(sf::Font* keyFont, sf::Font* labelFont);
    void update(float dt);
    void draw(sf::RenderWindow& window, AnimationController& anim, int badgeMode,
              int hoveredId, int selectedId, int targetId);

private:
    sf::Font* monoFont;
    sf::Font* smallFont;
    sf::VertexArray edges;
    sf::CircleShape ring;
    sf::CircleShape disc;
    sf::CircleShape glowCircle;
    sf::Text keyText;
    sf::Text badgeText;
    sf::Text labelText;
    float pulseClock;

    void addEdge(float x1, float y1, float x2, float y2, const sf::Color& color, float width);
    sf::Color stateRingColor(int state, bool& lit);
};
