#pragma once

#include <SFML/Graphics.hpp>

// Every colour and size the GUI uses, as static functions so both partners
// share one look. Functions, not variables, so nothing global exists.
class Theme
{
public:
    // ---- palette ----
    // Castle-at-night look: cold iron and near-black stone lit by torchlight.
    // Text stays close to parchment white so contrast against the dark panels
    // remains high - the theme is in the surfaces, never in the type.
    static sf::Color background() { return sf::Color(6, 7, 10); }
    static sf::Color gridLine() { return sf::Color(74, 62, 42, 26); }
    static sf::Color panel() { return sf::Color(20, 20, 26, 238); }
    static sf::Color panelLight() { return sf::Color(32, 31, 39, 238); }
    static sf::Color panelBorder() { return sf::Color(78, 65, 42); }
    static sf::Color text() { return sf::Color(237, 232, 221); }
    static sf::Color dimText() { return sf::Color(152, 144, 130); }
    static sf::Color balanced() { return sf::Color(104, 164, 108); }
    static sf::Color warning() { return sf::Color(216, 171, 64); }
    static sf::Color violation() { return sf::Color(198, 58, 52); }
    static sf::Color currentVisit() { return sf::Color(108, 158, 200); }
    static sf::Color skipped() { return sf::Color(80, 76, 70); }
    static sf::Color gold() { return sf::Color(201, 162, 77); }
    static sf::Color alarmLow() { return sf::Color(201, 162, 77); }
    static sf::Color alarmHigh() { return sf::Color(198, 58, 52); }

    // ---- rooms (vault doors) ----
    static sf::Color nodeRing() { return sf::Color(122, 111, 92); }
    static sf::Color nodeRingLit() { return sf::Color(208, 181, 120); }
    static sf::Color nodeDisc() { return sf::Color(16, 16, 21); }
    static sf::Color nodeDiscLit() { return sf::Color(31, 28, 31); }
    static sf::Color corridor() { return sf::Color(74, 65, 51); }
    static sf::Color corridorLit() { return sf::Color(206, 160, 70, 205); }

    // ---- hearthlight (the animated backdrop) ----
    static sf::Color ember() { return sf::Color(255, 176, 84); }
    static sf::Color emberDeep() { return sf::Color(214, 86, 34); }   // older, cooling ember
    static sf::Color hearthGlow() { return sf::Color(150, 84, 28); }
    static sf::Color torchGlow() { return sf::Color(196, 110, 34); }
    static sf::Color mist() { return sf::Color(78, 92, 116); }        // cold drifting haze
    static sf::Color ash() { return sf::Color(176, 186, 204); }       // pale motes falling

    // ---- sizes ----
    static float nodeRadius() { return 22.f; }
    static float panelRadius() { return 12.f; }
    static float corridorWidth() { return 3.f; }
    static float levelGapX() { return 64.f; }
    static float levelGapY() { return 92.f; }
    static int titleSize() { return 30; }
    static int headingSize() { return 20; }
    static int textSize() { return 16; }
    static int smallSize() { return 13; }
    static int monoSize() { return 14; }

    // ---- motion ----
    static float eventSeconds() { return 0.35f; }   // one event at 1x speed

    // smooth ease-in-out used by every animation
    static float ease(float t)
    {
        if (t < 0.f) { t = 0.f; }
        if (t > 1.f) { t = 1.f; }
        return t * t * (3.f - 2.f * t);
    }

    // linear blend between two colours
    static sf::Color mix(const sf::Color& a, const sf::Color& b, float t)
    {
        if (t < 0.f) { t = 0.f; }
        if (t > 1.f) { t = 1.f; }
        sf::Color c;
        c.r = (sf::Uint8)((float)a.r + ((float)b.r - (float)a.r) * t);
        c.g = (sf::Uint8)((float)a.g + ((float)b.g - (float)a.g) * t);
        c.b = (sf::Uint8)((float)a.b + ((float)b.b - (float)a.b) * t);
        c.a = (sf::Uint8)((float)a.a + ((float)b.a - (float)a.a) * t);
        return c;
    }

    static sf::Color withAlpha(const sf::Color& c, int alpha)
    {
        if (alpha < 0) { alpha = 0; }
        if (alpha > 255) { alpha = 255; }
        return sf::Color(c.r, c.g, c.b, (sf::Uint8)alpha);
    }
};
