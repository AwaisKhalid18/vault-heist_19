#include "gui/HeapTreeView.h"
#include "gui/Theme.h"
#include "core/TextUtil.h"
#include <cmath>

HeapTreeView::HeapTreeView()
{
    monoFont = nullptr;
    smallFont = nullptr;
    edges.setPrimitiveType(sf::Quads);
    pulseClock = 0.f;
    placeCount = 0;
}

void HeapTreeView::init(sf::Font* keyFont, sf::Font* labelFont)
{
    monoFont = keyFont;
    smallFont = labelFont;
    keyText.setFont(*keyFont);
    keyText.setCharacterSize(15);
    labelText.setFont(*labelFont);
    labelText.setCharacterSize(11);
}

void HeapTreeView::update(float dt)
{
    pulseClock += dt;
}

// The slot positions only depend on how many cells there are, so they are
// recomputed only when that count changes rather than once per lookup.
void HeapTreeView::refreshLayout(HeapAnimator& anim)
{
    int n = anim.getCount();
    if (n > Limits::MAX_NODES_GUI)
    {
        n = Limits::MAX_NODES_GUI;
    }
    if (n == placeCount)
    {
        return;
    }
    placeCount = layout.layoutCompleteTree(n, places);
}

// A swap slides the two cells along the straight line between their slots, so
// the player sees the exchange rather than a sudden jump.
bool HeapTreeView::positionOf(HeapAnimator& anim, int index, float& x, float& y)
{
    refreshLayout(anim);
    if (index < 0 || index >= placeCount)
    {
        return false;
    }
    x = places[index].x;
    y = places[index].y;
    if (!anim.isSwapping())
    {
        return true;
    }
    int a = anim.getHighlightA();
    int b = anim.getHighlightB();
    if (a < 0 || b < 0 || a >= placeCount || b >= placeCount)
    {
        return true;
    }
    float t = anim.getSwapProgress();
    if (index == a)
    {
        x = places[a].x + (places[b].x - places[a].x) * t;
        y = places[a].y + (places[b].y - places[a].y) * t;
    }
    else if (index == b)
    {
        x = places[b].x + (places[a].x - places[b].x) * t;
        y = places[b].y + (places[a].y - places[b].y) * t;
    }
    return true;
}

int HeapTreeView::indexAt(HeapAnimator& anim, float worldX, float worldY)
{
    float radius = Theme::nodeRadius() + 6.f;
    int n = anim.getCount();
    for (int i = n - 1; i >= 0; i--)
    {
        float x = 0.f;
        float y = 0.f;
        if (!positionOf(anim, i, x, y))
        {
            continue;
        }
        float dx = worldX - x;
        float dy = worldY - y;
        if (dx * dx + dy * dy <= radius * radius)
        {
            return i;
        }
    }
    return -1;
}

void HeapTreeView::getBounds(HeapAnimator& anim, float& minX, float& minY,
                             float& maxX, float& maxY)
{
    refreshLayout(anim);
    if (placeCount == 0)
    {
        minX = -120.f;
        minY = -120.f;
        maxX = 120.f;
        maxY = 120.f;
        return;
    }
    minX = places[0].x;
    maxX = places[0].x;
    minY = places[0].y;
    maxY = places[0].y;
    for (int i = 1; i < placeCount; i++)
    {
        if (places[i].x < minX) { minX = places[i].x; }
        if (places[i].x > maxX) { maxX = places[i].x; }
        if (places[i].y < minY) { minY = places[i].y; }
        if (places[i].y > maxY) { maxY = places[i].y; }
    }
}

void HeapTreeView::addEdge(float x1, float y1, float x2, float y2,
                           const sf::Color& color, float width)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    float length = sqrtf(dx * dx + dy * dy);
    if (length < 0.001f)
    {
        return;
    }
    // unit vector perpendicular to the corridor gives it its thickness
    float nx = -dy / length * width / 2.f;
    float ny = dx / length * width / 2.f;
    edges.append(sf::Vertex(sf::Vector2f(x1 + nx, y1 + ny), color));
    edges.append(sf::Vertex(sf::Vector2f(x2 + nx, y2 + ny), color));
    edges.append(sf::Vertex(sf::Vector2f(x2 - nx, y2 - ny), color));
    edges.append(sf::Vertex(sf::Vector2f(x1 - nx, y1 - ny), color));
}

void HeapTreeView::draw(sf::RenderWindow& window, HeapAnimator& anim,
                        int selectedIndex, int hoveredIndex)
{
    refreshLayout(anim);
    int n = anim.getCount();
    int heapSize = anim.getHeapSize();
    float radius = Theme::nodeRadius();

    // ---- corridors: only inside the active heap, since the sorted region
    // has left the tree and is no longer connected to it ----
    edges.clear();
    for (int i = 1; i < n; i++)
    {
        int parent = (i - 1) / 2;
        float cx = 0.f;
        float cy = 0.f;
        float px = 0.f;
        float py = 0.f;
        if (!positionOf(anim, i, cx, cy) || !positionOf(anim, parent, px, py))
        {
            continue;
        }
        bool bothActive = i < heapSize && parent < heapSize;
        sf::Color color = bothActive ? Theme::corridor() : Theme::withAlpha(Theme::skipped(), 90);
        addEdge(px, py, cx, cy, color, Theme::corridorWidth());
    }
    window.draw(edges);

    // ---- rooms ----
    for (int i = 0; i < n; i++)
    {
        float x = 0.f;
        float y = 0.f;
        if (!positionOf(anim, i, x, y))
        {
            continue;
        }
        bool active = i < heapSize;
        bool isRoot = i == 0 && heapSize > 0;
        bool isSwapped = anim.isSwapping()
                      && (i == anim.getHighlightA() || i == anim.getHighlightB());
        bool isHighlighted = i == anim.getHighlightA() || i == anim.getHighlightB();
        bool isActiveCell = i == anim.getActiveIndex();
        bool isSelected = i == selectedIndex;
        bool isHovered = i == hoveredIndex;

        // a parent that is beaten by one of its children is a live violation
        bool violates = false;
        if (active)
        {
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            if (left < heapSize && anim.getValue(left) > anim.getValue(i))
            {
                violates = true;
            }
            if (right < heapSize && anim.getValue(right) > anim.getValue(i))
            {
                violates = true;
            }
        }

        sf::Color ringColor = Theme::nodeRing();
        bool lit = false;
        if (!active)
        {
            ringColor = Theme::balanced();      // part of the finished sorted region
            lit = true;
        }
        if (violates)
        {
            ringColor = Theme::violation();
            lit = true;
        }
        if (isHighlighted || isActiveCell)
        {
            ringColor = Theme::currentVisit();
            lit = true;
        }
        if (isRoot && !violates && !isHighlighted)
        {
            ringColor = Theme::gold();
            lit = true;
        }
        if (isSelected)
        {
            ringColor = Theme::text();
            lit = true;
        }

        if (isHovered || lit)
        {
            glowCircle.setRadius(radius * 1.7f);
            glowCircle.setOrigin(radius * 1.7f, radius * 1.7f);
            glowCircle.setPosition(x, y);
            glowCircle.setFillColor(Theme::withAlpha(ringColor, isHovered ? 50 : 26));
            window.draw(glowCircle);
        }

        // pulsing ring on the cell the operation is working on, and on the root
        if (isActiveCell || isSwapped || (isRoot && !violates))
        {
            float pulse = 0.5f + 0.5f * sinf(pulseClock * 5.f);
            float pr = radius + 6.f + pulse * 4.f;
            ring.setRadius(pr);
            ring.setOrigin(pr, pr);
            ring.setPosition(x, y);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(2.f);
            ring.setOutlineColor(Theme::withAlpha(ringColor, (int)(120.f + 100.f * pulse)));
            window.draw(ring);
        }

        // the root gets a second ring: gold is never the only signal
        if (isRoot)
        {
            ring.setRadius(radius + 10.f);
            ring.setOrigin(radius + 10.f, radius + 10.f);
            ring.setPosition(x, y);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(2.f);
            ring.setOutlineColor(Theme::gold());
            window.draw(ring);
        }

        ring.setRadius(radius);
        ring.setOrigin(radius, radius);
        ring.setPosition(x, y);
        ring.setFillColor(lit ? Theme::nodeRingLit() : Theme::nodeRing());
        ring.setOutlineThickness(violates ? 4.f : 2.5f);
        ring.setOutlineColor(ringColor);
        window.draw(ring);

        float innerR = radius * 0.78f;
        disc.setRadius(innerR);
        disc.setOrigin(innerR, innerR);
        disc.setPosition(x, y);
        sf::Color discColor = lit ? Theme::nodeDiscLit() : Theme::nodeDisc();
        if (!active)
        {
            discColor = Theme::mix(discColor, Theme::balanced(), 0.25f);
        }
        disc.setFillColor(discColor);
        window.draw(disc);

        keyText.setString(TextUtil::intToText(anim.getValue(i)));
        sf::FloatRect kb = keyText.getLocalBounds();
        keyText.setPosition(x - kb.width / 2.f - kb.left, y - kb.height / 2.f - kb.top);
        keyText.setFillColor(active ? Theme::text() : Theme::dimText());
        window.draw(keyText);

        // the array index this room corresponds to, under the door
        labelText.setString("[" + TextUtil::intToText(i) + "]");
        sf::FloatRect ib = labelText.getLocalBounds();
        labelText.setPosition(x - ib.width / 2.f - ib.left, y + radius + 4.f);
        labelText.setFillColor(Theme::dimText());
        window.draw(labelText);

        // "!" over a parent its child beats, so red is never the only signal
        if (violates)
        {
            labelText.setCharacterSize(18);
            labelText.setString("!");
            sf::FloatRect vb = labelText.getLocalBounds();
            labelText.setPosition(x - vb.width / 2.f - vb.left, y - radius - 24.f);
            labelText.setFillColor(Theme::violation());
            window.draw(labelText);
            labelText.setCharacterSize(11);
        }

        // "sorted" under a room that has left the active heap
        if (!active)
        {
            labelText.setString("sorted");
            sf::FloatRect sb = labelText.getLocalBounds();
            labelText.setPosition(x - sb.width / 2.f - sb.left, y + radius + 18.f);
            labelText.setFillColor(Theme::balanced());
            window.draw(labelText);
        }

        // golden diamond over the root: it holds the maximum
        if (isRoot)
        {
            sf::ConvexShape diamond;
            diamond.setPointCount(4);
            float d = 7.f;
            float top = y - radius - 26.f;
            diamond.setPoint(0, sf::Vector2f(x, top - d));
            diamond.setPoint(1, sf::Vector2f(x + d, top));
            diamond.setPoint(2, sf::Vector2f(x, top + d));
            diamond.setPoint(3, sf::Vector2f(x - d, top));
            diamond.setFillColor(Theme::gold());
            window.draw(diamond);
        }
    }
}
