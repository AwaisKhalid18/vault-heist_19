#pragma once

#include <SFML/Graphics.hpp>

// The herald: a knight's bust in a gilded medallion, drawn beside a dialog
// so the briefing reads as somebody telling you the plan.
//
// Original artwork, built from primitives rather than loaded from a file -
// no portrait of any real person, and nothing extra to ship. If a file
// assets/ui/herald.png exists it is used instead, so a project that holds
// the rights to a picture can simply drop one in.
class KnightHerald
{
public:
    KnightHerald();

    // Draws the bust to fit the given box. `captionFont` may be null, in
    // which case the name plate underneath is skipped.
    void draw(sf::RenderWindow& window, float x, float y, float w, float h,
              sf::Font* captionFont);

private:
    sf::Texture fileTexture;      // the optional assets/ui/herald.png
    sf::RenderTexture bust;       // the painted version, drawn once
    bool triedLoad;
    bool hasFile;
    bool bustReady;

    sf::Clock clock;
    float elapsed;

    void ensureLoaded();
    void paintBust();             // fills `bust` with the knight
};
