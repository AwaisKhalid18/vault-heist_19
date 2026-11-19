#pragma once

#include "gui/Screen.h"
#include "gui/Background.h"
#include "gui/ParticleBurst.h"
#include "gui/widgets/Button.h"
#include "gui/widgets/RoundedRect.h"
#include "game/LevelDef.h"
#include "game/ScoreSystem.h"
#include "game/Campaign.h"

class App;

// The screen App::levelFinished opens in place of a level: stars, steps
// against the optimum, penalties and the score, then Retry, Next Level or
// back to the campaign map.
class ResultScreen : public Screen
{
public:
    ResultScreen(App& app, const string& levelId, const LevelOutcome& outcome);

    void onEnter();
    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    App& app;
    string levelId;
    LevelOutcome outcome;
    Campaign campaign;
    int levelIndex;
    int stars;
    int score;
    bool hasNext;

    Background background;
    ParticleBurst particles;
    RoundedRect panel;
    Button retryButton;
    Button nextButton;
    Button mapButton;
    sf::Text title;
    sf::Text subtitle;
    sf::Text body;
    sf::Text starText;
    float mouseX, mouseY;
    bool burstDone;

    void layout();
    void openLevel(int index);
    string buildBody();
};
