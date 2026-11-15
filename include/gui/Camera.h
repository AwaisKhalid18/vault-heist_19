#pragma once

#include <SFML/Graphics.hpp>

// The canvas camera: wheel zoom at the cursor, drag to pan, F to fit.
// All motion is smoothed towards target values every frame.
class Camera
{
public:
    Camera();

    // the screen rectangle (in window pixels) the canvas occupies
    void setViewport(float x, float y, float width, float height, float windowW, float windowH);

    void zoomAt(float wheelDelta, float pixelX, float pixelY);
    void startDrag(float pixelX, float pixelY);
    void dragTo(float pixelX, float pixelY);
    void endDrag();
    bool isDragging();
    bool didMove();            // did the last drag move more than a few pixels?
    void fitTo(float minX, float minY, float maxX, float maxY);   // world rectangle
    void update(float dt);
    void apply(sf::RenderWindow& window);                          // sets the view
    sf::Vector2f toWorld(sf::RenderWindow& window, float pixelX, float pixelY);
    bool viewportContains(float pixelX, float pixelY);

private:
    float centerX, centerY, zoom;
    float targetCenterX, targetCenterY, targetZoom;
    float vpX, vpY, vpW, vpH;
    float winW, winH;
    bool dragging;
    bool moved;
    float lastPixelX, lastPixelY;

    sf::View makeView(float cx, float cy, float z);
};
