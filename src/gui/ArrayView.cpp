#include "gui/ArrayView.h"
#include "gui/Theme.h"
#include "core/TextUtil.h"

ArrayView::ArrayView()
{
    valueFontPtr = nullptr;
    labelFontPtr = nullptr;
    posX = 0.f;
    posY = 0.f;
    sizeW = 800.f;
    sizeH = 96.f;
}

void ArrayView::init(sf::Font* valueFont, sf::Font* labelFont)
{
    valueFontPtr = valueFont;
    labelFontPtr = labelFont;
    valueText.setFont(*valueFont);
    valueText.setCharacterSize(15);
    indexText.setFont(*valueFont);
    indexText.setCharacterSize(11);
    labelText.setFont(*labelFont);
    labelText.setCharacterSize(12);
}

void ArrayView::setRect(float x, float y, float width, float height)
{
    posX = x;
    posY = y;
    sizeW = width;
    sizeH = height;
}

// Cells shrink to fit however many values there are, down to a floor that
// still leaves room for a three-digit number.
float ArrayView::cellWidth(HeapAnimator& anim)
{
    int n = anim.getCount();
    if (n <= 0)
    {
        return 54.f;
    }
    float available = sizeW - 24.f;
    float width = available / (float)n;
    if (width > 54.f)
    {
        width = 54.f;
    }
    if (width < 22.f)
    {
        width = 22.f;
    }
    return width;
}

float ArrayView::rowLeft(HeapAnimator& anim)
{
    float width = cellWidth(anim);
    float total = width * (float)anim.getCount();
    return posX + (sizeW - total) / 2.f;
}

float ArrayView::rowTop()
{
    return posY + 28.f;
}

bool ArrayView::cellRect(HeapAnimator& anim, int index, float& x, float& y, float& w, float& h)
{
    int n = anim.getCount();
    if (index < 0 || index >= n)
    {
        return false;
    }
    float width = cellWidth(anim);
    float left = rowLeft(anim);
    x = left + width * (float)index;
    y = rowTop();
    w = width;
    h = 40.f;

    // a swap slides the two cells across each other, matching the tree view
    if (anim.isSwapping())
    {
        int a = anim.getHighlightA();
        int b = anim.getHighlightB();
        if (a >= 0 && b >= 0 && a < n && b < n)
        {
            float t = anim.getSwapProgress();
            float ax = left + width * (float)a;
            float bx = left + width * (float)b;
            if (index == a)
            {
                x = ax + (bx - ax) * t;
                y = y - 10.f * t * (1.f - t) * 4.f;   // small arc so they pass visibly
            }
            else if (index == b)
            {
                x = bx + (ax - bx) * t;
                y = y + 10.f * t * (1.f - t) * 4.f;
            }
        }
    }
    return true;
}

int ArrayView::indexAt(HeapAnimator& anim, float pixelX, float pixelY)
{
    int n = anim.getCount();
    for (int i = 0; i < n; i++)
    {
        float x = 0.f;
        float y = 0.f;
        float w = 0.f;
        float h = 0.f;
        if (!cellRect(anim, i, x, y, w, h))
        {
            continue;
        }
        if (pixelX >= x && pixelX <= x + w && pixelY >= y && pixelY <= y + h)
        {
            return i;
        }
    }
    return -1;
}

void ArrayView::draw(sf::RenderWindow& window, HeapAnimator& anim,
                     int selectedIndex, int hoveredIndex)
{
    int n = anim.getCount();
    int heapSize = anim.getHeapSize();
    float width = cellWidth(anim);
    float left = rowLeft(anim);
    float top = rowTop();

    // ---- the two region labels, drawn above the cells they describe ----
    labelText.setFillColor(Theme::currentVisit());
    labelText.setString("ACTIVE HEAP [0.." + TextUtil::intToText(heapSize - 1) + "]");
    if (heapSize <= 0)
    {
        labelText.setString("ACTIVE HEAP (empty)");
    }
    labelText.setPosition(left, posY + 4.f);
    window.draw(labelText);

    if (n > heapSize)
    {
        labelText.setFillColor(Theme::balanced());
        labelText.setString("SORTED [" + TextUtil::intToText(heapSize) + ".."
                            + TextUtil::intToText(n - 1) + "]");
        sf::FloatRect lb = labelText.getLocalBounds();
        float labelX = left + width * (float)heapSize;
        float rowRight = left + width * (float)n;
        if (labelX + lb.width > rowRight)
        {
            labelX = rowRight - lb.width;
        }
        labelText.setPosition(labelX, posY + 4.f);
        window.draw(labelText);
    }

    // ---- the cells ----
    for (int i = 0; i < n; i++)
    {
        float x = 0.f;
        float y = 0.f;
        float w = 0.f;
        float h = 0.f;
        if (!cellRect(anim, i, x, y, w, h))
        {
            continue;
        }
        bool active = i < heapSize;
        bool isHighlighted = i == anim.getHighlightA() || i == anim.getHighlightB();
        bool isActiveCell = i == anim.getActiveIndex();
        bool isRoot = i == 0 && heapSize > 0;
        bool isSelected = i == selectedIndex;
        bool isHovered = i == hoveredIndex;

        sf::Color border = Theme::panelBorder();
        sf::Color fill = Theme::panelLight();
        if (!active)
        {
            border = Theme::balanced();
            fill = Theme::mix(Theme::panelLight(), Theme::balanced(), 0.18f);
        }
        if (isRoot)
        {
            border = Theme::gold();
        }
        if (isHighlighted || isActiveCell)
        {
            border = Theme::currentVisit();
            fill = Theme::mix(fill, Theme::currentVisit(), 0.25f);
        }
        if (isSelected)
        {
            border = Theme::text();
            fill = Theme::mix(fill, Theme::text(), 0.15f);
        }
        if (isHovered)
        {
            fill = Theme::mix(fill, Theme::text(), 0.1f);
        }

        cell.setPosition(x + 2.f, y);
        cell.setSize(sf::Vector2f(w - 4.f, h));
        cell.setFillColor(fill);
        cell.setOutlineThickness(isHighlighted || isSelected ? 2.5f : 1.f);
        cell.setOutlineColor(border);
        window.draw(cell);

        valueText.setString(TextUtil::intToText(anim.getValue(i)));
        if (w < 34.f)
        {
            valueText.setCharacterSize(11);
        }
        else
        {
            valueText.setCharacterSize(15);
        }
        sf::FloatRect vb = valueText.getLocalBounds();
        valueText.setPosition(x + (w - vb.width) / 2.f - vb.left,
                              y + (h - vb.height) / 2.f - vb.top);
        valueText.setFillColor(active ? Theme::text() : Theme::dimText());
        window.draw(valueText);

        // the index under each cell, which is what ties it to the tree view
        indexText.setString(TextUtil::intToText(i));
        sf::FloatRect ib = indexText.getLocalBounds();
        indexText.setPosition(x + (w - ib.width) / 2.f - ib.left, y + h + 4.f);
        indexText.setFillColor(Theme::dimText());
        window.draw(indexText);
    }

    // ---- the bracket marking where the active heap ends ----
    if (heapSize > 0 && heapSize < n)
    {
        float boundary = left + width * (float)heapSize;
        cell.setPosition(boundary - 1.f, top - 6.f);
        cell.setSize(sf::Vector2f(2.f, 52.f));
        cell.setFillColor(Theme::warning());
        cell.setOutlineThickness(0.f);
        window.draw(cell);
    }

    if (anim.isSortComplete())
    {
        labelText.setFillColor(Theme::gold());
        labelText.setString("FINAL VAULT UNLOCKED - array sorted ascending");
        sf::FloatRect cb = labelText.getLocalBounds();
        labelText.setPosition(posX + (sizeW - cb.width) / 2.f, top + 58.f);
        window.draw(labelText);
    }
}
