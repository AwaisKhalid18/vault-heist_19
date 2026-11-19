#pragma once

#include "gui/Screen.h"
#include "gui/Background.h"
#include "gui/Camera.h"
#include "gui/HeapAnimator.h"
#include "gui/HeapTreeView.h"
#include "gui/ArrayView.h"
#include "gui/widgets/Hud.h"
#include "gui/widgets/Button.h"
#include "gui/widgets/OperationPanel.h"
#include "gui/widgets/RoundedRect.h"
#include "gui/widgets/Dialog.h"
#include "model/MaxHeap.h"
#include "core/Random.h"

class App;

// Endless survival on the Heap Fortress.
//
// Treasures keep arriving - an insert - and the player survives by extracting
// maxima to keep the active heap under its limit. The validator runs after
// every single operation, and the run ends the moment the vault overflows.
// The score is how long the vault held, which is what the spec asks of
// endless mode: mixed inserts and removals, survival time while the
// structure stays valid, and a delivery rate that ramps up.
class EndlessScreen : public Screen
{
public:
    EndlessScreen(App& app);

    void onEnter();
    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    static const int ST_READY = 0, ST_RUNNING = 1, ST_OVER = 2;
    static const int OP_EXTRACT = 0, OP_HEAPIFY = 1, OP_START = 2;
    static const int CAPACITY = 24;         // the vault overflows past this

    App& app;
    Background background;
    Camera camera;
    MaxHeap heap;
    EventLog opLog;
    HeapAnimator anim;
    HeapTreeView heapView;
    ArrayView arrayView;
    Hud hud;
    OperationPanel opPanel;
    Dialog dialog;
    RoundedRect infoPanel;
    Random rng;

    int state;
    float survivedSeconds;
    float nextDeliveryIn;
    float deliveryInterval;
    int delivered;
    int extracted;
    int score;
    sf::Text line;
    float mouseX, mouseY;
    float rightX, rightW;
    bool ready;

    void layout();
    void startRun();
    void deliverTreasure();
    void extractOnce();
    void endRun(const string& reason);
    void showHeap();
    int liveScore();
};
