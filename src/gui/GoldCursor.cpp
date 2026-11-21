#include "gui/GoldCursor.h"
#include <cmath>

// final cursor size, the supersample factor, and the working size
static const int CUR_W = 32;
static const int CUR_H = 32;
static const int SS = 3;
static const int HI_W = CUR_W * SS;
static const int HI_H = CUR_H * SS;

// how far the outline reaches outside the arrow, in high-resolution cells
static const int OUTLINE = 4;

// The arrow: a sharp pointer with a bladed tail swept to the lower right.
// Point 0 is the tip, which sits on the hot spot. Coordinates are in final
// pixels and are shifted by TIP so the outline around the tip is not
// clipped by the edge of the bitmap.
static const float TIP = 2.f;
static const int POINT_COUNT = 7;
static const float ARROW_X[POINT_COUNT] = { 0.f,  0.f,  5.6f,  9.8f, 14.4f, 10.4f, 18.2f };
static const float ARROW_Y[POINT_COUNT] = { 0.f, 24.4f, 18.9f, 28.8f, 26.6f, 17.3f, 17.3f };

static bool insideArrow(float x, float y)
{
    // ray casting: count the edges crossed going right from (x, y)
    bool in = false;
    int j = POINT_COUNT - 1;
    for (int i = 0; i < POINT_COUNT; i++)
    {
        float xi = ARROW_X[i] + TIP;
        float yi = ARROW_Y[i] + TIP;
        float xj = ARROW_X[j] + TIP;
        float yj = ARROW_Y[j] + TIP;
        bool straddles = (yi > y) != (yj > y);
        if (straddles)
        {
            float cross = (xj - xi) * (y - yi) / (yj - yi) + xi;
            if (x < cross)
            {
                in = !in;
            }
        }
        j = i;
    }
    return in;
}

bool GoldCursor::apply(sf::RenderWindow& window, sf::Cursor& out)
{
    // 1. solid mask at high resolution
    static bool inside[HI_W * HI_H];
    for (int y = 0; y < HI_H; y++)
    {
        for (int x = 0; x < HI_W; x++)
        {
            float fx = ((float)x + 0.5f) / (float)SS;
            float fy = ((float)y + 0.5f) / (float)SS;
            inside[y * HI_W + x] = insideArrow(fx, fy);
        }
    }

    // 2. outline: anything just outside the mask
    static bool edge[HI_W * HI_H];
    for (int y = 0; y < HI_H; y++)
    {
        for (int x = 0; x < HI_W; x++)
        {
            edge[y * HI_W + x] = false;
            if (inside[y * HI_W + x])
            {
                continue;
            }
            bool near = false;
            for (int dy = -OUTLINE; dy <= OUTLINE && !near; dy++)
            {
                for (int dx = -OUTLINE; dx <= OUTLINE && !near; dx++)
                {
                    if (dx * dx + dy * dy > OUTLINE * OUTLINE)
                    {
                        continue;
                    }
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx < 0 || ny < 0 || nx >= HI_W || ny >= HI_H)
                    {
                        continue;
                    }
                    if (inside[ny * HI_W + nx])
                    {
                        near = true;
                    }
                }
            }
            edge[y * HI_W + x] = near;
        }
    }

    // 3. average each 3x3 block down into one final pixel
    static sf::Uint8 pixels[CUR_W * CUR_H * 4];
    for (int y = 0; y < CUR_H; y++)
    {
        for (int x = 0; x < CUR_W; x++)
        {
            float fillSum = 0.f;
            float edgeSum = 0.f;
            float rSum = 0.f, gSum = 0.f, bSum = 0.f;

            for (int sy = 0; sy < SS; sy++)
            {
                for (int sx = 0; sx < SS; sx++)
                {
                    int hx = x * SS + sx;
                    int hy = y * SS + sy;
                    if (inside[hy * HI_W + hx])
                    {
                        fillSum += 1.f;
                        // gilded sheen: bright along the leading edge,
                        // deepening towards the tail
                        float t = ((float)hx / (float)HI_W) * 0.45f
                                + ((float)hy / (float)HI_H) * 0.55f;
                        if (t > 1.f) { t = 1.f; }
                        rSum += 255.f - 70.f * t;
                        gSum += 216.f - 92.f * t;
                        bSum += 138.f - 96.f * t;
                    }
                    else if (edge[hy * HI_W + hx])
                    {
                        edgeSum += 1.f;
                    }
                }
            }

            float total = (float)(SS * SS);
            float fillCov = fillSum / total;
            float edgeCov = edgeSum / total;
            int index = (y * CUR_W + x) * 4;

            if (fillSum <= 0.f && edgeSum <= 0.f)
            {
                pixels[index + 0] = 0;
                pixels[index + 1] = 0;
                pixels[index + 2] = 0;
                pixels[index + 3] = 0;
                continue;
            }

            float r = 26.f, g = 20.f, b = 12.f;    // outline colour
            if (fillSum > 0.f)
            {
                float fr = rSum / fillSum;
                float fg = gSum / fillSum;
                float fb = bSum / fillSum;
                // blend the gold over the dark edge by their coverages
                float w = fillCov / (fillCov + edgeCov);
                r = fr * w + r * (1.f - w);
                g = fg * w + g * (1.f - w);
                b = fb * w + b * (1.f - w);
            }

            float alpha = (fillCov + edgeCov) * 255.f;
            if (alpha > 255.f) { alpha = 255.f; }

            pixels[index + 0] = (sf::Uint8)r;
            pixels[index + 1] = (sf::Uint8)g;
            pixels[index + 2] = (sf::Uint8)b;
            pixels[index + 3] = (sf::Uint8)alpha;
        }
    }

    if (!out.loadFromPixels(pixels, sf::Vector2u(CUR_W, CUR_H),
                            sf::Vector2u((unsigned int)TIP, (unsigned int)TIP)))
    {
        return false;
    }
    window.setMouseCursor(out);
    return true;
}
