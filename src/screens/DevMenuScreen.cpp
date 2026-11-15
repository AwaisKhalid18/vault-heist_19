#include "screens/DevMenuScreen.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "game/LevelLoader.h"
#include "game/LevelFactory.h"
#include "screens/SandboxScreen.h"
#include "screens/RotationDemoScreen.h"
#include "screens/StressScreen.h"
#include <iostream>

DevMenuScreen::DevMenuScreen(App& a) : app(a)
{
    levelCount = 0;
    mouseX = 0.f;
    mouseY = 0.f;
    layoutDone = false;
}

void DevMenuScreen::onEnter()
{
    title.setFont(app.titleFont);
    title.setCharacterSize(46);
    title.setFillColor(Theme::gold());
    title.setString("VAULT HEIST");
    subtitle.setFont(app.uiFont);
    subtitle.setCharacterSize(Theme::textSize());
    subtitle.setFillColor(Theme::dimText());
    subtitle.setString("The Living Vault  -  Partner A dev launcher");
    footer.setFont(app.uiFont);
    footer.setCharacterSize(Theme::smallSize());
    footer.setFillColor(Theme::dimText());
    footer.setString("Keys 1-4 open the LL / RR / LR / RL rotation demos   |   F12 screenshot   |   Esc quits");

    // load the campaign list once
    if (levelCount == 0)
    {
        string paths[MAX_LEVELS];
        int pathCount = LevelLoader::loadLevelList("assets/levels/levels.txt", paths, MAX_LEVELS);
        for (int i = 0; i < pathCount; i++)
        {
            LevelDef def;
            if (LevelLoader::load(paths[i], def))
            {
                levelDefs[levelCount] = def;
                levelCount++;
            }
        }
    }
    layout();
}

void DevMenuScreen::layout()
{
    sf::Vector2u size = app.window.getSize();
    float w = (float)size.x;
    float centerX = w / 2.f;

    sandboxButton.init(&app.uiFont, "Sandbox", centerX - 330.f, 220.f, 200.f, 48.f);
    sandboxButton.setAccent(true);
    stressButton.init(&app.uiFont, "200-node stress view", centerX - 110.f, 220.f, 220.f, 48.f);
    quitButton.init(&app.uiFont, "Quit", centerX + 130.f, 220.f, 200.f, 48.f);

    string rotationNames[4] = { "Demo: LL", "Demo: RR", "Demo: LR", "Demo: RL" };
    for (int i = 0; i < 4; i++)
    {
        rotationButtons[i].init(&app.uiFont, rotationNames[i],
                                centerX - 330.f + (float)i * 170.f, 286.f, 150.f, 42.f);
    }

    float y = 370.f;
    for (int i = 0; i < levelCount; i++)
    {
        float x = centerX - 330.f + (float)(i % 2) * 340.f;
        float by = y + (float)(i / 2) * 54.f;
        levelButtons[i].init(&app.uiFont, levelDefs[i].id + "  " + levelDefs[i].title,
                             x, by, 320.f, 44.f);
    }
    layoutDone = true;
}

void DevMenuScreen::openLevel(int index)
{
    Screen* screen = LevelFactory::create(app, levelDefs[index]);
    if (screen != nullptr)
    {
        app.push(screen);
    }
    else
    {
        cout << "Level type " << levelDefs[index].type << " is not available yet (TODO(B))" << endl;
    }
}

void DevMenuScreen::openRotationDemo(int rotCase)
{
    app.push(new RotationDemoScreen(app, rotCase));
}

void DevMenuScreen::handleEvent(const sf::Event& e)
{
    if (e.type == sf::Event::MouseMoved)
    {
        mouseX = (float)e.mouseMove.x;
        mouseY = (float)e.mouseMove.y;
    }
    if (e.type == sf::Event::Resized)
    {
        layout();
    }
    if (e.type == sf::Event::KeyPressed)
    {
        if (e.key.code == sf::Keyboard::Escape)
        {
            app.quit();
            return;
        }
        if (e.key.code == sf::Keyboard::Num1) { openRotationDemo(Event::ROT_LL); return; }
        if (e.key.code == sf::Keyboard::Num2) { openRotationDemo(Event::ROT_RR); return; }
        if (e.key.code == sf::Keyboard::Num3) { openRotationDemo(Event::ROT_LR); return; }
        if (e.key.code == sf::Keyboard::Num4) { openRotationDemo(Event::ROT_RL); return; }
    }
    if (sandboxButton.handleEvent(e))
    {
        app.push(new SandboxScreen(app));
        return;
    }
    if (stressButton.handleEvent(e))
    {
        app.push(new StressScreen(app));
        return;
    }
    if (quitButton.handleEvent(e))
    {
        app.quit();
        return;
    }
    for (int i = 0; i < 4; i++)
    {
        if (rotationButtons[i].handleEvent(e))
        {
            openRotationDemo(Event::ROT_LL + i);
            return;
        }
    }
    for (int i = 0; i < levelCount; i++)
    {
        if (levelButtons[i].handleEvent(e))
        {
            openLevel(i);
            return;
        }
    }
}

void DevMenuScreen::update(float dt)
{
    if (!layoutDone)
    {
        layout();
    }
    sandboxButton.update(dt, mouseX, mouseY);
    stressButton.update(dt, mouseX, mouseY);
    quitButton.update(dt, mouseX, mouseY);
    for (int i = 0; i < 4; i++)
    {
        rotationButtons[i].update(dt, mouseX, mouseY);
    }
    for (int i = 0; i < levelCount; i++)
    {
        levelButtons[i].update(dt, mouseX, mouseY);
    }
}

void DevMenuScreen::draw(sf::RenderWindow& window)
{
    app.useUiView();
    background.draw(window);
    sf::Vector2u size = window.getSize();
    float centerX = (float)size.x / 2.f;

    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition(centerX - tb.width / 2.f, 90.f);
    window.draw(title);
    sf::FloatRect sb = subtitle.getLocalBounds();
    subtitle.setPosition(centerX - sb.width / 2.f, 160.f);
    window.draw(subtitle);

    sandboxButton.draw(window);
    stressButton.draw(window);
    quitButton.draw(window);
    for (int i = 0; i < 4; i++)
    {
        rotationButtons[i].draw(window);
    }
    for (int i = 0; i < levelCount; i++)
    {
        levelButtons[i].draw(window);
    }

    sf::FloatRect fb = footer.getLocalBounds();
    footer.setPosition(centerX - fb.width / 2.f, (float)size.y - 44.f);
    window.draw(footer);
}
