#pragma once

#include "gui/Screen.h"
#include "gui/Background.h"
#include "gui/widgets/Button.h"
#include "gui/widgets/Slider.h"
#include "gui/widgets/RoundedRect.h"

class App;

// Settings, all stored in the save file: animation speed, whether the
// validator runs after every operation, whether tooltips appear, unlocking
// every level for a demo, and wiping the progress.
class SettingsScreen : public Screen
{
public:
    SettingsScreen(App& app);

    void onEnter();
    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    App& app;
    Background background;
    RoundedRect panel;
    Slider speedSlider;
    Button validateButton;
    Button tooltipButton;
    Button unlockButton;
    Button resetButton;
    Button backButton;
    sf::Text title;
    sf::Text label;
    float mouseX, mouseY;
    bool confirmingReset;
    bool ready;

    void layout();
    void refreshLabels();
    void storeAndSave();
    float sliderToSpeed();
};
