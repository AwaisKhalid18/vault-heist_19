#include "gui/TreeRenderer.h"
#include "gui/Theme.h"
#include "core/TextUtil.h"
#include <cmath>

TreeRenderer::TreeRenderer()
{
    monoFont = nullptr;
    smallFont = nullptr;
    edges.setPrimitiveType(sf::Quads);
    pulseClock = 0.f;
}

void TreeRenderer::init(sf::Font* keyFont, sf::Font* labelFont)
{
    monoFont = keyFont;
    smallFont = labelFont;
    keyText.setFont(*keyFont);
    keyText.setCharacterSize(15);
    badgeText.setFont(*keyFont);
    badgeText.setCharacterSize(11);
    labelText.setFont(*labelFont);
    labelText.setCharacterSize(11);
}

void TreeRenderer::update(float dt)
{
    pulseClock += dt;
}

void TreeRenderer::addEdge(float x1, float y1, float x2, float y2, const sf::Color& color, float width)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    float length = sqrtf(dx * dx + dy * dy);
    if (length < 0.001f)
    {
        return;
    }
    // unit vector perpendicular to the corridor gives it thickness
    float nx = -dy / length * width / 2.f;
    float ny = dx / length * width / 2.f;
    edges.append(sf::Vertex(sf::Vector2f(x1 + nx, y1 + ny), color));
    edges.append(sf::Vertex(sf::Vector2f(x2 + nx, y2 + ny), color));
    edges.append(sf::Vertex(sf::Vector2f(x2 - nx, y2 - ny), color));
    edges.append(sf::Vertex(sf::Vector2f(x1 - nx, y1 - ny), color));
}

sf::Color TreeRenderer::stateRingColor(int state, bool& lit)
{
    lit = true;
    if (state == AnimationController::STATE_VISITED) { return Theme::currentVisit(); }
    if (state == AnimationController::STATE_CURRENT) { return Theme::currentVisit(); }
    if (state == AnimationController::STATE_RESULT) { return Theme::gold(); }
    if (state == AnimationController::STATE_TARGET) { return Theme::gold(); }
    if (state == AnimationController::STATE_VIOLATION) { return Theme::violation(); }
    if (state == AnimationController::STATE_WARNING) { return Theme::warning(); }
    if (state == AnimationController::STATE_SELECTED) { return Theme::text(); }
    if (state == AnimationController::STATE_SKIPPED)
    {
        lit = false;
        return Theme::skipped();
    }
    lit = false;
    return Theme::nodeRing();
}

void TreeRenderer::draw(sf::RenderWindow& window, AnimationController& anim, int badgeMode,
                        int hoveredId, int selectedId, int targetId)
{
    NodeVisual* nodes = anim.getVisuals();
    int count = anim.getVisualCount();
    float radius = Theme::nodeRadius();
    bool manyNodes = count > 100;   // cheaper drawing for the stress view

    // ---- corridors ----
    edges.clear();
    for (int i = 0; i < count; i++)
    {
        NodeVisual& v = nodes[i];
        if (v.parentId == -1)
        {
            continue;
        }
        NodeVisual* parent = anim.findById(v.parentId);
        if (parent == nullptr)
        {
            continue;
        }
        float alpha = v.alpha < parent->alpha ? v.alpha : parent->alpha;
        bool vLit = v.state == AnimationController::STATE_VISITED
                 || v.state == AnimationController::STATE_CURRENT
                 || v.state == AnimationController::STATE_RESULT;
        bool pLit = parent->state == AnimationController::STATE_VISITED
                 || parent->state == AnimationController::STATE_CURRENT
                 || parent->state == AnimationController::STATE_RESULT;
        if (vLit && pLit)
        {
            // glowing corridor: a wide soft quad under a bright core
            sf::Color glow = Theme::withAlpha(Theme::corridorLit(), (int)(alpha * 0.25f));
            addEdge(parent->x, parent->y, v.x, v.y, glow, Theme::corridorWidth() * 4.f);
            addEdge(parent->x, parent->y, v.x, v.y,
                    Theme::withAlpha(Theme::corridorLit(), (int)alpha), Theme::corridorWidth());
        }
        else
        {
            addEdge(parent->x, parent->y, v.x, v.y,
                    Theme::withAlpha(Theme::corridor(), (int)alpha), Theme::corridorWidth());
        }
    }
    window.draw(edges);

    // ---- rooms ----
    for (int i = 0; i < count; i++)
    {
        NodeVisual& v = nodes[i];
        float r = radius * v.scale;
        if (r < 1.f || v.alpha < 3.f)
        {
            continue;
        }
        int alpha = (int)v.alpha;
        bool lit = false;
        sf::Color ringColor = stateRingColor(v.state, lit);
        bool isHovered = v.id == hoveredId;
        bool isSelected = v.id == selectedId;
        bool isTarget = v.id == targetId;
        if (isTarget)
        {
            ringColor = Theme::gold();
            lit = true;
        }
        if (isSelected)
        {
            ringColor = Theme::text();
        }

        // hover glow halo
        if ((isHovered || lit) && !manyNodes)
        {
            glowCircle.setRadius(r * 1.7f);
            glowCircle.setOrigin(r * 1.7f, r * 1.7f);
            glowCircle.setPosition(v.x, v.y);
            glowCircle.setFillColor(Theme::withAlpha(ringColor, isHovered ? 50 : 26));
            window.draw(glowCircle);
        }

        // pulsing ring around the current room (and the target)
        if (v.state == AnimationController::STATE_CURRENT || isTarget)
        {
            float pulse = 0.5f + 0.5f * sinf(pulseClock * 5.f);
            float pr = r + 6.f + pulse * 4.f;
            ring.setRadius(pr);
            ring.setOrigin(pr, pr);
            ring.setPosition(v.x, v.y);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(2.f);
            ring.setOutlineColor(Theme::withAlpha(ringColor, (int)(120.f + 100.f * pulse)));
            window.draw(ring);
        }

        // the target's second ring (double ring = never colour alone)
        if (isTarget)
        {
            ring.setRadius(r + 10.f);
            ring.setOrigin(r + 10.f, r + 10.f);
            ring.setPosition(v.x, v.y);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(2.f);
            ring.setOutlineColor(Theme::withAlpha(Theme::gold(), alpha));
            window.draw(ring);
        }

        // metallic outer ring (the vault door)
        ring.setRadius(r);
        ring.setOrigin(r, r);
        ring.setPosition(v.x, v.y);
        ring.setFillColor(Theme::withAlpha(lit ? Theme::nodeRingLit() : Theme::nodeRing(), alpha));
        ring.setOutlineThickness(v.state == AnimationController::STATE_VIOLATION ? 4.f : 2.5f);
        ring.setOutlineColor(Theme::withAlpha(ringColor, alpha));
        window.draw(ring);

        // darker inner disc
        float innerR = r * 0.78f;
        disc.setRadius(innerR);
        disc.setOrigin(innerR, innerR);
        disc.setPosition(v.x, v.y);
        sf::Color discColor = lit ? Theme::nodeDiscLit() : Theme::nodeDisc();
        if (v.state == AnimationController::STATE_SKIPPED)
        {
            discColor = Theme::mix(discColor, Theme::skipped(), 0.35f);
        }
        if (v.flash > 0.f)
        {
            discColor = Theme::mix(discColor, Theme::gold(), v.flash * 0.6f);
        }
        disc.setFillColor(Theme::withAlpha(discColor, alpha));
        window.draw(disc);

        // key number in the centre
        keyText.setString(TextUtil::intToText(v.key));
        sf::FloatRect kb = keyText.getLocalBounds();
        keyText.setPosition(v.x - kb.width / 2.f - kb.left, v.y - kb.height / 2.f - kb.top);
        sf::Color keyColor = v.state == AnimationController::STATE_SKIPPED ? Theme::dimText() : Theme::text();
        keyText.setFillColor(Theme::withAlpha(keyColor, alpha));
        window.draw(keyText);

        // badge (height / BF / size) at the top right of the door
        if (badgeMode != BADGE_NONE)
        {
            int value = v.heightVal;
            sf::Color badgeColor = Theme::dimText();
            if (badgeMode == BADGE_BF)
            {
                value = v.bf;
                if (v.bf > 1 || v.bf < -1) { badgeColor = Theme::violation(); }
                else if (v.bf != 0) { badgeColor = Theme::warning(); }
                else { badgeColor = Theme::balanced(); }
            }
            else if (badgeMode == BADGE_SIZE)
            {
                value = v.sizeVal;
                badgeColor = Theme::gold();
            }
            float bx = v.x + r * 0.95f;
            float by = v.y - r * 1.25f;
            disc.setRadius(10.f);
            disc.setOrigin(10.f, 10.f);
            disc.setPosition(bx, by);
            disc.setFillColor(Theme::withAlpha(sf::Color(16, 21, 32), alpha));
            window.draw(disc);
            ring.setRadius(10.f);
            ring.setOrigin(10.f, 10.f);
            ring.setPosition(bx, by);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(v.flash > 0.f ? 2.5f : 1.f);
            ring.setOutlineColor(Theme::withAlpha(badgeColor, alpha));
            window.draw(ring);
            badgeText.setString(TextUtil::intToText(value));
            sf::FloatRect bb = badgeText.getLocalBounds();
            badgeText.setPosition(bx - bb.width / 2.f - bb.left, by - bb.height / 2.f - bb.top);
            badgeText.setFillColor(Theme::withAlpha(badgeColor, alpha));
            window.draw(badgeText);
        }

        // "!" above a violated room: the red colour is never alone
        if (v.state == AnimationController::STATE_VIOLATION)
        {
            labelText.setCharacterSize(18);
            labelText.setString("!");
            sf::FloatRect lb = labelText.getLocalBounds();
            labelText.setPosition(v.x - lb.width / 2.f - lb.left, v.y - r - 24.f);
            labelText.setFillColor(Theme::withAlpha(Theme::violation(), alpha));
            window.draw(labelText);
            labelText.setCharacterSize(11);
        }

        // "pruned" below a skipped room
        if (v.state == AnimationController::STATE_SKIPPED && !manyNodes)
        {
            labelText.setString("pruned");
            sf::FloatRect lb = labelText.getLocalBounds();
            labelText.setPosition(v.x - lb.width / 2.f - lb.left, v.y + r + 6.f);
            labelText.setFillColor(Theme::withAlpha(Theme::dimText(), alpha));
            window.draw(labelText);
        }

        // golden diamond above the target room
        if (isTarget)
        {
            sf::ConvexShape diamond;
            diamond.setPointCount(4);
            float d = 7.f;
            diamond.setPoint(0, sf::Vector2f(v.x, v.y - r - 26.f - d));
            diamond.setPoint(1, sf::Vector2f(v.x + d, v.y - r - 26.f));
            diamond.setPoint(2, sf::Vector2f(v.x, v.y - r - 26.f + d));
            diamond.setPoint(3, sf::Vector2f(v.x - d, v.y - r - 26.f));
            diamond.setFillColor(Theme::withAlpha(Theme::gold(), alpha));
            window.draw(diamond);
        }
    }
}
