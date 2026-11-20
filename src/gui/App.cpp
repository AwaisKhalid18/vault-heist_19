#include "gui/App.h"
#include "gui/Theme.h"
#include "core/TextUtil.h"
#include "screens/ResultScreen.h"
#include <iostream>

App::App()
{
    screenCount = 0;
    debugValidate = false;
    wantQuit = false;
    screenshotCounter = 0;
    pendingAction = PENDING_NONE;
    pendingScreen = nullptr;
    pendingDepth = 0;
    savePath = "vault_heist_save.txt";
}

App::~App()
{
    while (screenCount > 0)
    {
        screenCount--;
        delete screens[screenCount];
    }
    delete pendingScreen;      // a change requested but never applied
}

bool App::init(const string& windowTitle)
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    window.create(sf::VideoMode(1600, 900), windowTitle, sf::Style::Default, settings);
    window.setVerticalSyncEnabled(true);

    // assets are copied next to the exe and also live in the repo root,
    // so try both places for each font
    if (!titleFont.loadFromFile("assets/fonts/Cinzel.ttf")
        && !titleFont.loadFromFile("../assets/fonts/Cinzel.ttf"))
    {
        cout << "Missing assets/fonts/Cinzel.ttf" << endl;
        return false;
    }
    if (!uiFont.loadFromFile("assets/fonts/Inter.ttf")
        && !uiFont.loadFromFile("../assets/fonts/Inter.ttf"))
    {
        cout << "Missing assets/fonts/Inter.ttf" << endl;
        return false;
    }
    if (!monoFont.loadFromFile("assets/fonts/JetBrainsMono.ttf")
        && !monoFont.loadFromFile("../assets/fonts/JetBrainsMono.ttf"))
    {
        cout << "Missing assets/fonts/JetBrainsMono.ttf" << endl;
        return false;
    }
    return true;
}

void App::push(Screen* screen)
{
    if (screenCount >= Limits::MAX_SCREENS)
    {
        delete screen;
        return;
    }
    screens[screenCount] = screen;
    screenCount++;
    screen->onEnter();
}

void App::pop()
{
    if (screenCount == 0)
    {
        return;
    }
    screenCount--;
    delete screens[screenCount];
    if (screenCount > 0)
    {
        screens[screenCount - 1]->onEnter();   // returning screen refreshes itself
    }
}

void App::replaceTop(Screen* screen)
{
    if (screenCount > 0)
    {
        screenCount--;
        delete screens[screenCount];
    }
    push(screen);
}

Screen* App::top()
{
    if (screenCount == 0)
    {
        return nullptr;
    }
    return screens[screenCount - 1];
}

int App::stackSize()
{
    return screenCount;
}

void App::quit()
{
    wantQuit = true;
}

void App::useUiView()
{
    sf::Vector2u size = window.getSize();
    window.setView(sf::View(sf::FloatRect(0.f, 0.f, (float)size.x, (float)size.y)));
}

void App::requestPop()
{
    pendingAction = PENDING_POP;
}

void App::requestReplaceTop(Screen* screen)
{
    if (pendingScreen != nullptr)
    {
        delete pendingScreen;      // a second request in one frame wins
    }
    pendingAction = PENDING_REPLACE;
    pendingScreen = screen;
}

void App::requestPopToDepth(int depth)
{
    pendingAction = PENDING_POP_TO;
    pendingDepth = depth;
}

void App::applyPendingChange()
{
    int action = pendingAction;
    pendingAction = PENDING_NONE;
    if (action == PENDING_POP)
    {
        pop();
    }
    else if (action == PENDING_REPLACE)
    {
        Screen* screen = pendingScreen;
        pendingScreen = nullptr;
        replaceTop(screen);
    }
    else if (action == PENDING_POP_TO)
    {
        while (screenCount > pendingDepth && screenCount > 0)
        {
            pop();
        }
    }
}

// Records the result, writes the save file and opens the Result screen in
// place of the level. The change is requested rather than done here, because
// this is called from inside the level screen's own event handling.
void App::levelFinished(const string& levelId, const LevelOutcome& o)
{
    int stars = ScoreSystem::computeStars(o);
    int score = ScoreSystem::computeScore(o);
    if (o.completed)
    {
        save.recordLevel(levelId, stars, score);
        save.save(savePath);
    }
    requestReplaceTop(new ResultScreen(*this, levelId, o));
}

void App::saveScreenshot()
{
    sf::Vector2u size = window.getSize();
    sf::Texture texture;
    if (!texture.create(size.x, size.y))
    {
        return;
    }
    texture.update(window);
    sf::Image image = texture.copyToImage();
    screenshotCounter++;
    string path = "screenshots/screenshot_" + TextUtil::intToText(screenshotCounter) + ".png";
    if (image.saveToFile(path))
    {
        cout << "Saved " << path << endl;
    }
}

void App::run()
{
    sf::Clock clock;
    while (window.isOpen() && !wantQuit && screenCount > 0)
    {
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed)
            {
                window.close();
                break;
            }
            if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::F12)
            {
                saveScreenshot();
                continue;
            }
            if (e.type == sf::Event::Resized)
            {
                // keep 1 unit = 1 pixel when the window is resized
                sf::FloatRect visible(0.f, 0.f, (float)e.size.width, (float)e.size.height);
                window.setView(sf::View(visible));
            }
            if (screenCount > 0)
            {
                screens[screenCount - 1]->handleEvent(e);
            }
            // a screen that asked to be replaced or popped is still running
            // its handler above, so the change is applied only now
            applyPendingChange();
            if (screenCount == 0)
            {
                break;
            }
        }
        if (!window.isOpen() || wantQuit || screenCount == 0)
        {
            break;
        }

        float dt = clock.restart().asSeconds();
        if (dt > 0.1f)
        {
            dt = 0.1f;   // avoid huge jumps after a stall
        }
        screens[screenCount - 1]->update(dt);
        applyPendingChange();
        if (!window.isOpen() || wantQuit || screenCount == 0)
        {
            break;
        }

        window.clear(Theme::background());
        screens[screenCount - 1]->draw(window);
        window.display();
    }
    if (window.isOpen())
    {
        window.close();
    }
}
