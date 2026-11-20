#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "core/Limits.h"
#include "gui/Screen.h"
#include "game/ScoreSystem.h"
#include "game/SaveManager.h"

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

    // A screen often changes the stack from inside its own event handler.
    // Popping or replacing it there would delete the object whose method is
    // still running, so these park the change and run() applies it between
    // frames instead. Pushing is safe either way.
    void requestPop();
    void requestReplaceTop(Screen* screen);
    void requestPopToDepth(int depth);     // unwind back to a menu

    // Selects a 1:1 pixel view covering the whole window. Screens use this
    // for panels and text instead of getDefaultView(), which keeps the size
    // the window had at creation and so breaks when the window is resized.
    void useUiView();

    // Records the result, writes the save file and swaps the level for the
    // Result screen. The swap is deferred, because this is called from inside
    // the level screen's own event handler.
    void levelFinished(const string& levelId, const LevelOutcome& o);

    sf::RenderWindow window;
    sf::Font titleFont;
    sf::Font uiFont;
    sf::Font monoFont;
    bool debugValidate;

    // Campaign progress, the local leaderboard and the settings. It lives on
    // the App because every menu and every level outcome touches it.
    SaveManager save;
    string savePath;

private:
    static const int PENDING_NONE = 0, PENDING_POP = 1, PENDING_REPLACE = 2, PENDING_POP_TO = 3;

    Screen* screens[Limits::MAX_SCREENS];
    int screenCount;
    bool wantQuit;
    int screenshotCounter;

    int pendingAction;
    Screen* pendingScreen;
    int pendingDepth;

    void saveScreenshot();
    void applyPendingChange();
};
