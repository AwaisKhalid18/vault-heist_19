#include "gui/Camera.h"

Camera::Camera()
{
    centerX = 0.f;
    centerY = 200.f;
    zoom = 1.f;
    targetCenterX = centerX;
    targetCenterY = centerY;
    targetZoom = zoom;
    vpX = 0.f;
    vpY = 0.f;
    vpW = 1120.f;
    vpH = 900.f;
    winW = 1600.f;
    winH = 900.f;
    dragging = false;
    moved = false;
    lastPixelX = 0.f;
    lastPixelY = 0.f;
}

void Camera::setViewport(float x, float y, float width, float height, float windowW, float windowH)
{
    vpX = x;
    vpY = y;
    vpW = width;
    vpH = height;
    winW = windowW;
    winH = windowH;
}

sf::View Camera::makeView(float cx, float cy, float z)
{
    sf::View view;
    view.setCenter(cx, cy);
    view.setSize(vpW / z, vpH / z);
    view.setViewport(sf::FloatRect(vpX / winW, vpY / winH, vpW / winW, vpH / winH));
    return view;
}

bool Camera::viewportContains(float pixelX, float pixelY)
{
    return pixelX >= vpX && pixelX <= vpX + vpW && pixelY >= vpY && pixelY <= vpY + vpH;
}

sf::Vector2f Camera::toWorld(sf::RenderWindow& window, float pixelX, float pixelY)
{
    return window.mapPixelToCoords(sf::Vector2i((int)pixelX, (int)pixelY), makeView(centerX, centerY, zoom));
}

void Camera::zoomAt(float wheelDelta, float pixelX, float pixelY)
{
    float factor = wheelDelta > 0.f ? 1.18f : 1.f / 1.18f;
    float newZoom = targetZoom * factor;
    if (newZoom < 0.15f) { newZoom = 0.15f; }
    if (newZoom > 4.f) { newZoom = 4.f; }

    // keep the world point under the cursor fixed while zooming
    float worldX = targetCenterX + (pixelX - (vpX + vpW / 2.f)) / targetZoom;
    float worldY = targetCenterY + (pixelY - (vpY + vpH / 2.f)) / targetZoom;
    targetCenterX = worldX - (pixelX - (vpX + vpW / 2.f)) / newZoom;
    targetCenterY = worldY - (pixelY - (vpY + vpH / 2.f)) / newZoom;
    targetZoom = newZoom;
}

void Camera::startDrag(float pixelX, float pixelY)
{
    dragging = true;
    moved = false;
    lastPixelX = pixelX;
    lastPixelY = pixelY;
}

void Camera::dragTo(float pixelX, float pixelY)
{
    if (!dragging)
    {
        return;
    }
    float dx = pixelX - lastPixelX;
    float dy = pixelY - lastPixelY;
    if (dx * dx + dy * dy > 9.f)
    {
        moved = true;
    }
    targetCenterX -= (pixelX - lastPixelX) / zoom;
    targetCenterY -= (pixelY - lastPixelY) / zoom;
    // panning should not lag behind the hand
    centerX = targetCenterX;
    centerY = targetCenterY;
    lastPixelX = pixelX;
    lastPixelY = pixelY;
}

void Camera::endDrag()
{
    dragging = false;
}

bool Camera::isDragging()
{
    return dragging;
}

bool Camera::didMove()
{
    return moved;
}

void Camera::fitTo(float minX, float minY, float maxX, float maxY)
{
    float margin = 80.f;
    float width = maxX - minX + margin * 2.f;
    float height = maxY - minY + margin * 2.f;
    if (width < 1.f) { width = 1.f; }
    if (height < 1.f) { height = 1.f; }
    float zx = vpW / width;
    float zy = vpH / height;
    float z = zx < zy ? zx : zy;
    if (z > 1.5f) { z = 1.5f; }
    if (z < 0.15f) { z = 0.15f; }
    targetZoom = z;
    targetCenterX = (minX + maxX) / 2.f;
    targetCenterY = (minY + maxY) / 2.f;
}

void Camera::update(float dt)
{
    float speed = 10.f * dt;
    if (speed > 1.f)
    {
        speed = 1.f;
    }
    centerX += (targetCenterX - centerX) * speed;
    centerY += (targetCenterY - centerY) * speed;
    zoom += (targetZoom - zoom) * speed;
}

void Camera::apply(sf::RenderWindow& window)
{
    window.setView(makeView(centerX, centerY, zoom));
}
