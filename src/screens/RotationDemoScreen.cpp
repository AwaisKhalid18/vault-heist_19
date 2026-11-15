#include "screens/RotationDemoScreen.h"
#include "gui/App.h"
#include "gui/Theme.h"

RotationDemoScreen::RotationDemoScreen(App& a, int rc) : app(a)
{
    rotCase = rc;
}

void RotationDemoScreen::onEnter()
{
    renderer.init(&app.monoFont, &app.uiFont);
    logPanel.init(&app.titleFont, &app.monoFont);
    caption.setFont(app.titleFont);
    caption.setCharacterSize(Theme::titleSize());
    caption.setFillColor(Theme::gold());
    start(rotCase);
}

void RotationDemoScreen::start(int newCase)
{
    rotCase = newCase;
    tree.clear();
    tree.setEventLog(nullptr);

    // two keys prepare the shape, the third triggers the chosen case
    int first = 0;
    int second = 0;
    int trigger = 0;
    string name = "";
    if (rotCase == Event::ROT_LL) { first = 30; second = 20; trigger = 10; name = "LL: right rotation"; }
    else if (rotCase == Event::ROT_RR) { first = 10; second = 20; trigger = 30; name = "RR: left rotation"; }
    else if (rotCase == Event::ROT_LR) { first = 30; second = 10; trigger = 20; name = "LR: left then right"; }
    else { rotCase = Event::ROT_RL; first = 10; second = 30; trigger = 20; name = "RL: right then left"; }
    caption.setString(name + "   (press R to replay, 1-4 to switch, Esc to go back)");

    tree.insert(first);
    tree.insert(second);
    log.clear();
    log.saveTreeShape(tree.getRoot());
    tree.setEventLog(&log);
    tree.insert(trigger);
    tree.setEventLog(nullptr);
    anim.setSpeed(0.6f);
    anim.startLog(&log, true);
    camera.fitTo(-200.f, -50.f, 200.f, 300.f);
}

void RotationDemoScreen::handleEvent(const sf::Event& e)
{
    if (e.type == sf::Event::KeyPressed)
    {
        if (e.key.code == sf::Keyboard::Escape)
        {
            app.pop();
            return;
        }
        if (e.key.code == sf::Keyboard::R) { start(rotCase); }
        if (e.key.code == sf::Keyboard::Num1) { start(Event::ROT_LL); }
        if (e.key.code == sf::Keyboard::Num2) { start(Event::ROT_RR); }
        if (e.key.code == sf::Keyboard::Num3) { start(Event::ROT_LR); }
        if (e.key.code == sf::Keyboard::Num4) { start(Event::ROT_RL); }
        if (e.key.code == sf::Keyboard::Space) { anim.togglePlay(); }
        if (e.key.code == sf::Keyboard::Left) { anim.stepBack(); }
        if (e.key.code == sf::Keyboard::Right) { anim.stepForward(); }
    }
    logPanel.handleEvent(e);
}

void RotationDemoScreen::update(float dt)
{
    sf::Vector2u size = app.window.getSize();
    camera.setViewport(0.f, 70.f, (float)size.x - 380.f, (float)size.y - 70.f,
                       (float)size.x, (float)size.y);
    logPanel.setRect((float)size.x - 370.f, 90.f, 350.f, (float)size.y - 180.f);
    camera.update(dt);
    anim.update(dt);
    renderer.update(dt);
}

void RotationDemoScreen::draw(sf::RenderWindow& window)
{
    background.draw(window);
    camera.apply(window);
    renderer.draw(window, anim, TreeRenderer::BADGE_HEIGHT, -1, -1, -1);
    window.setView(window.getDefaultView());
    caption.setPosition(30.f, 18.f);
    window.draw(caption);
    logPanel.draw(window, anim.getLog(), anim.currentIndex());
}
