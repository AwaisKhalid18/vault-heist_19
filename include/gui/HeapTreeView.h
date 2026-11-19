#pragma once

#include <SFML/Graphics.hpp>
#include "gui/HeapAnimator.h"
#include "gui/LayoutEngine.h"

// Draws the Max Heap as a binary tree of vault doors, in the same style as
// TreeRenderer draws Acts 1-3. Positions come from
// LayoutEngine::layoutCompleteTree, because a heap is always a complete tree
// and index i's children are always at 2i+1 and 2i+2.
//
// Everything it draws is read from the HeapAnimator, which is what keeps it
// in step with the array view below it.
class HeapTreeView
{
public:
    HeapTreeView();

    void init(sf::Font* keyFont, sf::Font* labelFont);
    void update(float dt);
    void draw(sf::RenderWindow& window, HeapAnimator& anim, int selectedIndex, int hoveredIndex);

    // Where index i is drawn right now, including a swap in progress.
    bool positionOf(HeapAnimator& anim, int index, float& x, float& y);
    int indexAt(HeapAnimator& anim, float worldX, float worldY);
    void getBounds(HeapAnimator& anim, float& minX, float& minY, float& maxX, float& maxY);

private:
    sf::Font* monoFont;
    sf::Font* smallFont;
    sf::VertexArray edges;
    sf::CircleShape ring;
    sf::CircleShape disc;
    sf::CircleShape glowCircle;
    sf::Text keyText;
    sf::Text labelText;
    float pulseClock;
    LayoutEngine layout;
    NodePlacement places[Limits::MAX_NODES_GUI];
    int placeCount;

    void refreshLayout(HeapAnimator& anim);
    void addEdge(float x1, float y1, float x2, float y2, const sf::Color& color, float width);
};
