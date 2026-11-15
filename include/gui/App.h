#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "core/Limits.h"
#include "gui/Screen.h"
#include "game/ScoreSystem.h"

using namespace std;

// Owns the window, the fonts and the screen stack.
// Created as a local variable in main(), never as a global.
class App
{
public:
    App();
    ~App();

    bool init(const string& windowTitle);        // window + fonts; false if a font is missing
    void run();                                  // events -> update(dt) -> draw

    void push(Screen* screen);
    void pop();
    void replaceTop(Screen* screen);
    Screen* top();
    int stackSize();
    void quit();

    // Phase 0 behaviour: print the outcome and pop the level.
    // TODO(B): open the Result screen instead.
    void levelFinished(const string& levelId, const LevelOutcome& o);

    sf::RenderWindow window;
    sf::Font titleFont;
    sf::Font uiFont;
    sf::Font monoFont;
    bool debugValidate;

private:
    Screen* screens[Limits::MAX_SCREENS];
    int screenCount;
    bool wantQuit;
    int screenshotCounter;

    void saveScreenshot();
};
