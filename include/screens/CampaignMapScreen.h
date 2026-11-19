#pragma once

#include "gui/Screen.h"
#include "gui/Background.h"
#include "gui/widgets/Button.h"
#include "gui/widgets/RoundedRect.h"
#include "game/Campaign.h"

class App;

// The campaign map: four acts side by side, each with its five levels, the
// stars earned on each, and a lock on anything the player has not reached.
class CampaignMapScreen : public Screen
{
public:
    CampaignMapScreen(App& app);

    void onEnter();
    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    App& app;
    Background background;
    Campaign campaign;
    Button levelButtons[Campaign::MAX_CAMPAIGN_LEVELS];
    Button backButton;
    RoundedRect actPanels[4];
    sf::Text title;
    sf::Text actTitle;
    sf::Text starLine;
    sf::Text footer;
    float mouseX, mouseY;
    bool unlocked[Campaign::MAX_CAMPAIGN_LEVELS];
    bool ready;

    void layout();
    void refreshUnlocks();
    void openLevel(int index);
    string actName(int act);
};
