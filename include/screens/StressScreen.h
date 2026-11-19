#pragma once

#include "gui/Screen.h"
#include "gui/Background.h"
#include "gui/Camera.h"
#include "gui/TreeRenderer.h"
#include "gui/AnimationController.h"
#include "model/AVLTree.h"

class App;

// A 200-node AVL tree for checking that zoom and pan hold 60 fps.
class StressScreen : public Screen
{
public:
    StressScreen(App& app);

    void onEnter();
    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    App& app;
    Background background;
    Camera camera;
    TreeRenderer renderer;
    AnimationController anim;
    AVLTree tree;
    EventLog log;
    sf::Text info;
    float fpsTimer;
    int frameCount;
    int fps;
};
