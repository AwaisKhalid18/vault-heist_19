#pragma once

#include <SFML/Graphics.hpp>

// Every colour and size the GUI uses, as static functions so both partners
// share one look. Functions, not variables, so nothing global exists.
class Theme
{
public:
    // ---- palette ----
    static sf::Color background() { return sf::Color(11, 15, 23); }
    static sf::Color gridLine() { return sf::Color(42, 52, 72, 40); }
    static sf::Color panel() { return sf::Color(21, 28, 42, 235); }
    static sf::Color panelLight() { return sf::Color(30, 39, 58, 235); }
    static sf::Color panelBorder() { return sf::Color(42, 52, 72); }
    static sf::Color text() { return sf::Color(230, 233, 239); }
    static sf::Color dimText() { return sf::Color(138, 147, 166); }
    static sf::Color balanced() { return sf::Color(63, 185, 122); }
    static sf::Color warning() { return sf::Color(234, 217, 76); }
    static sf::Color violation() { return sf::Color(229, 72, 77); }
    static sf::Color currentVisit() { return sf::Color(76, 154, 255); }
    static sf::Color skipped() { return sf::Color(74, 82, 99); }
    static sf::Color gold() { return sf::Color(212, 165, 55); }
    static sf::Color alarmLow() { return sf::Color(212, 165, 55); }
    static sf::Color alarmHigh() { return sf::Color(229, 72, 77); }

    // ---- rooms (vault doors) ----
    static sf::Color nodeRing() { return sf::Color(106, 115, 133); }
    static sf::Color nodeRingLit() { return sf::Color(160, 172, 196); }
    static sf::Color nodeDisc() { return sf::Color(16, 21, 32); }
    static sf::Color nodeDiscLit() { return sf::Color(24, 31, 46); }
    static sf::Color corridor() { return sf::Color(52, 62, 84); }
    static sf::Color corridorLit() { return sf::Color(76, 154, 255, 200); }

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
