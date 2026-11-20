#pragma once

#include <SFML/Graphics.hpp>
#include "gui/widgets/Button.h"
#include "gui/widgets/Slider.h"
#include "gui/widgets/RoundedRect.h"

// Bottom bar: play controls, Undo / Replay / Reset / Hint, the speed
// slider and the replay scrubber. handleEvent returns an ACT_ constant.
class Toolbar
{
public:
    static const int ACT_NONE = -1, ACT_PLAY = 0, ACT_STEP_BACK = 1, ACT_STEP_FWD = 2,
                     ACT_UNDO = 3, ACT_REPLAY = 4, ACT_RESET = 5, ACT_HINT = 6,
                     ACT_SPEED = 7, ACT_SCRUB = 8, ACT_FIT = 9;

    Toolbar();

    void init(sf::Font* font);
    void setRect(float x, float y, float width, float height);
    int handleEvent(const sf::Event& e);
    void update(float dt, float mouseX, float mouseY, bool playing, float progress);
    void draw(sf::RenderWindow& window);

    float getSpeed();            // 0.25 .. 4
    float getScrub();            // 0 .. 1 (valid right after ACT_SCRUB)

private:
    RoundedRect panel;
    Button playButton;
    Button backButton;
    Button fwdButton;
    Button undoButton;
    Button replayButton;
    Button resetButton;
    Button hintButton;
    Button fitButton;
    Slider speedSlider;
    Slider scrubSlider;
    sf::Text speedLabel;
    sf::Text scrubLabel;
    float posX, posY, sizeW, sizeH;
    float speedLabelX, speedLabelY;
    bool scrubbed;
    bool ready;
    sf::Font* font;
};
