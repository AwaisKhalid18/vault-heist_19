#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "gui/widgets/RoundedRect.h"

using namespace std;

// Top-right panel: objective, animated alarm meter, score and steps.
// The meter fills smoothly, blends amber to red and pulses near the top.
class Hud
{
public:
    Hud();

    void init(sf::Font* headingFont, sf::Font* textFont);
    void setRect(float x, float y, float width, float height);
    void setObjective(const string& text);
    void setAlarm(int steps, int capacity);
    void setScoreInfo(int score, int steps, int optimal);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    RoundedRect panel;
    sf::Text title;
    sf::Text objective;
    sf::Text alarmLabel;
    sf::Text scoreText;
    float posX, posY, sizeW, sizeH;
    int alarmSteps, alarmCapacity;
    int scoreValue, stepsValue, optimalValue;
    float shownFill;       // animated 0..1 meter fill
    float pulseClock;
};
