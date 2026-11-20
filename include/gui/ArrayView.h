#pragma once

#include <SFML/Graphics.hpp>
#include "gui/HeapAnimator.h"

// The row of array cells under the Act 4 heap: the underlying storage the
// tree view is really drawing. It reads the same HeapAnimator the tree view
// reads, so the two can never drift apart.
//
// The active heap and the sorted region are told apart by a label and a
// bracket as well as by colour, which the spec's accessibility rule requires.
class ArrayView
{
public:
    ArrayView();

    void init(sf::Font* valueFont, sf::Font* labelFont);
    void setRect(float x, float y, float width, float height);
    void draw(sf::RenderWindow& window, HeapAnimator& anim, int selectedIndex, int hoveredIndex);

    // Which cell is under this window pixel, or -1. The row is drawn in
    // screen coordinates, so it does not move when the canvas is panned.
    int indexAt(HeapAnimator& anim, float pixelX, float pixelY);
    bool cellRect(HeapAnimator& anim, int index, float& x, float& y, float& w, float& h);

private:
    sf::Font* valueFontPtr;
    sf::Font* labelFontPtr;
    sf::Text valueText;
    sf::Text indexText;
    sf::Text labelText;
    sf::RectangleShape cell;
    float posX, posY, sizeW, sizeH;

    float cellWidth(HeapAnimator& anim);
    float rowLeft(HeapAnimator& anim);
    float rowTop();
};
