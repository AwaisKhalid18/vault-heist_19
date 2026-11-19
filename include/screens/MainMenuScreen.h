#pragma once

#include "gui/Screen.h"
#include "gui/Background.h"
#include "gui/widgets/Button.h"
#include "game/Campaign.h"

class App;

// The game's front door: Campaign, Endless, Sandbox, Leaderboard, Settings
// and Quit, with the progress loaded from the save file underneath it.
class MainMenuScreen : public Screen
{
public:
    MainMenuScreen(App& app);

    void onEnter();
    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    static const int BUTTON_COUNT = 6;

    App& app;
    Background background;
    Button buttons[BUTTON_COUNT];
    Campaign campaign;
    sf::Text title;
    sf::Text subtitle;
    sf::Text progressText;
    sf::Text footer;
    float mouseX, mouseY;
    float glowClock;
    bool ready;

    void layout();
    void openCampaign();
};
