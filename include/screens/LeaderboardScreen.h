#pragma once

#include "gui/Screen.h"
#include "gui/Background.h"
#include "gui/widgets/Button.h"
#include "gui/widgets/RoundedRect.h"
#include "game/Campaign.h"

class App;

// The local leaderboard: the best Endless runs on this machine, and the
// campaign record beside them so one screen answers "how am I doing".
class LeaderboardScreen : public Screen
{
public:
    LeaderboardScreen(App& app);

    void onEnter();
    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    App& app;
    Background background;
    Campaign campaign;
    RoundedRect runsPanel;
    RoundedRect campaignPanel;
    Button backButton;
    sf::Text title;
    sf::Text heading;
    sf::Text line;
    float mouseX, mouseY;
    bool ready;

    void layout();
};
