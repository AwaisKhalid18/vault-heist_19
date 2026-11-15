#include "screens/StressScreen.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "core/Random.h"
#include "core/TextUtil.h"

StressScreen::StressScreen(App& a) : app(a)
{
    fpsTimer = 0.f;
    frameCount = 0;
    fps = 0;
}

void StressScreen::onEnter()
{
    renderer.init(&app.monoFont, &app.uiFont);
    info.setFont(app.uiFont);
    info.setCharacterSize(Theme::textSize());
    info.setFillColor(Theme::text());

    // 200 distinct seeded keys
    tree.clear();
    int keys[200];
    for (int i = 0; i < 200; i++)
    {
        keys[i] = i + 1;
    }
    Random rng(20200);
    rng.shuffle(keys, 200);
    for (int i = 0; i < 200; i++)
    {
        tree.insert(keys[i]);
    }

    log.clear();
    log.saveTreeShape(tree.getRoot());
    anim.startLog(&log, false);

    float minX, minY, maxX, maxY;
    anim.getBounds(minX, minY, maxX, maxY);
    camera.fitTo(minX, minY, maxX, maxY);
}

void StressScreen::handleEvent(const sf::Event& e)
{
    if (e.type == sf::Event::KeyPressed)
    {
        if (e.key.code == sf::Keyboard::Escape)
        {
            app.pop();
            return;
        }
        if (e.key.code == sf::Keyboard::F)
        {
            float minX, minY, maxX, maxY;
            anim.getBounds(minX, minY, maxX, maxY);
            camera.fitTo(minX, minY, maxX, maxY);
        }
    }
    if (e.type == sf::Event::MouseWheelScrolled)
    {
        camera.zoomAt(e.mouseWheelScroll.delta,
                      (float)e.mouseWheelScroll.x, (float)e.mouseWheelScroll.y);
    }
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
    {
        camera.startDrag((float)e.mouseButton.x, (float)e.mouseButton.y);
    }
    if (e.type == sf::Event::MouseButtonReleased)
    {
        camera.endDrag();
    }
    if (e.type == sf::Event::MouseMoved)
    {
        camera.dragTo((float)e.mouseMove.x, (float)e.mouseMove.y);
    }
}

void StressScreen::update(float dt)
{
    sf::Vector2u size = app.window.getSize();
    camera.setViewport(0.f, 0.f, (float)size.x, (float)size.y, (float)size.x, (float)size.y);
    camera.update(dt);
    anim.update(dt);
    renderer.update(dt);

    frameCount++;
    fpsTimer += dt;
    if (fpsTimer >= 0.5f)
    {
        fps = (int)((float)frameCount / fpsTimer + 0.5f);
        frameCount = 0;
        fpsTimer = 0.f;
    }
    info.setString("200-node AVL stress view   |   FPS " + TextUtil::intToText(fps)
                   + "   |   wheel zoom, drag pan, F fit, Esc back");
}

void StressScreen::draw(sf::RenderWindow& window)
{
    app.useUiView();
    background.draw(window);
    camera.apply(window);
    renderer.draw(window, anim, TreeRenderer::BADGE_NONE, -1, -1, -1);
    app.useUiView();
    info.setPosition(24.f, 18.f);
    window.draw(info);
}
