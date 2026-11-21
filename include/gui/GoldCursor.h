#pragma once

#include <SFML/Graphics.hpp>

// The pointer: a gilded arrow with a bladed tail and a dark outline, drawn
// into a bitmap at start-up instead of shipping an image file.
//
// Rendered at three times the final size and averaged back down, so the
// diagonals come out smooth rather than stepped.
class GoldCursor
{
public:
    // Fills `out` and hands it to the window. Returns false if the system
    // refuses the bitmap, in which case the usual arrow stays.
    static bool apply(sf::RenderWindow& window, sf::Cursor& out);
};
