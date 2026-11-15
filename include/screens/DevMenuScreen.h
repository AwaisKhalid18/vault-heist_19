#pragma once

#include "gui/Screen.h"
#include "gui/Background.h"
#include "gui/widgets/Button.h"
#include "game/LevelDef.h"

class App;

// Partner A's launcher: sandbox, a picker for every Act 2-3 level,
// the four rotation demos (keys 1-4) and the 200-node stress view.
class DevMenuScreen : public Screen
{
public:
    DevMenuScreen(App& app);

    void onEnter();
    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    static const int MAX_LEVELS = 24;

    App& app;
    Background background;
    sf::Text title;
    sf::Text subtitle;
    sf::Text footer;
    Button sandboxButton;
    Button stressButton;
    Button quitButton;
    Button rotationButtons[4];
    Button levelButtons[MAX_LEVELS];
    LevelDef levelDefs[MAX_LEVELS];
    int levelCount;
    float mouseX, mouseY;
    bool layoutDone;

    void layout();
    void openLevel(int index);
    void openRotationDemo(int rotCase);
};
