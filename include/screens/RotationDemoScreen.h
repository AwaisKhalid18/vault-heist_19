#pragma once

#include "gui/Screen.h"
#include "gui/Background.h"
#include "gui/Camera.h"
#include "gui/TreeRenderer.h"
#include "gui/AnimationController.h"
#include "gui/widgets/EventLogPanel.h"
#include "model/AVLTree.h"

class App;

// Shows one rotation case (LL / RR / LR / RL) as a replayable animation,
// built for taking the report screenshots. R replays, 1-4 switch cases.
class RotationDemoScreen : public Screen
{
public:
    RotationDemoScreen(App& app, int rotCase);

    void onEnter();
    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    App& app;
    int rotCase;
    Background background;
    Camera camera;
    TreeRenderer renderer;
    AnimationController anim;
    EventLogPanel logPanel;
    AVLTree tree;
    EventLog log;
    sf::Text caption;

    void start(int newCase);
};
