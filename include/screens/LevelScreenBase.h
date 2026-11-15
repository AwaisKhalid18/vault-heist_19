#pragma once

#include "gui/Screen.h"
#include "gui/Background.h"
#include "gui/Camera.h"
#include "gui/TreeRenderer.h"
#include "gui/AnimationController.h"
#include "gui/ParticleBurst.h"
#include "gui/widgets/Hud.h"
#include "gui/widgets/Toolbar.h"
#include "gui/widgets/EventLogPanel.h"
#include "gui/widgets/HintPanel.h"
#include "gui/widgets/Dialog.h"
#include "gui/widgets/Tooltip.h"
#include "game/LevelDef.h"
#include "game/ScoreSystem.h"
#include "model/AVLTree.h"

class App;

// Everything the ten Act 2-3 levels share: briefing dialog, canvas with
// camera and animation, HUD with the alarm meter, toolbar, event log,
// hints, undo, replay, win/lose flow and the call to app.levelFinished.
// Each level subclass only adds its own interaction on top.
class LevelScreenBase : public Screen
{
public:
    LevelScreenBase(App& app, const LevelDef& def);
    virtual ~LevelScreenBase();

    void onEnter();
    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window);

protected:
    static const int ST_BRIEFING = 0, ST_PLAY = 1, ST_WON = 2, ST_LOST = 3;
    static const int DLG_NONE = 0, DLG_BRIEFING = 1, DLG_WIN = 2, DLG_LOSE = 3, DLG_CUSTOM = 4;

    App& app;
    LevelDef def;
    AVLTree tree;
    EventLog opLog;
    AnimationController anim;
    TreeRenderer renderer;
    Camera camera;
    Background background;
    ParticleBurst particles;
    Hud hud;
    Toolbar toolbar;
    EventLogPanel logPanel;
    HintPanel hintPanel;
    Dialog dialog;
    Tooltip tooltip;

    int state;
    int dialogKind;
    int alarmSteps, capacity, optimalSteps, penaltyCount, penaltyPoints, bonuses;
    bool hintUsed;
    int hintIndex;
    int badgeMode;
    int hoveredId, selectedId, targetId;
    float mouseX, mouseY;
    string objective;
    AVLTree undoCopy;
    bool hasUndo;
    bool animWasFinished;

    // layout of the right column, for subclasses that add their own panel
    float rightX, rightW, subPanelY, subPanelH;

    // ---- services for the subclasses ----
    void setObjective(const string& text);
    void showTreeStatic(bool fitCamera);
    void beginOperation();                 // snapshot + route events into opLog
    void endOperation(bool autoPlay);      // detach log and play the animation
    int countVisits();                     // VISIT events in the current opLog
    void addAlarm(int steps);
    void penalty(int alarmPlus, int points, const string& message);
    void win(const string& message);
    void lose(const string& message);
    void saveUndo();
    void restoreUndo();
    void fitCameraToTree();
    void restartLevel();
    void openCustomDialog(const string& title, const string& body,
                          const string labels[], int count);
    LevelOutcome makeOutcome(bool completed);
    sf::Vector2f worldMouse();

    // ---- hooks each level fills in ----
    virtual void onLevelStart() = 0;       // build the tree, set objective + optimal
    virtual void onNodeClicked(int nodeId);
    virtual void onLevelEvent(const sf::Event& e);
    virtual void onLevelUpdate(float dt);
    virtual void onAnimationDone();
    virtual void onUndoRestored();
    virtual void onDialogChoice(int button);           // for DLG_CUSTOM dialogs
    virtual void drawCanvasOverlay(sf::RenderWindow& window);   // world coords
    virtual void drawUiOverlay(sf::RenderWindow& window);       // screen coords
    virtual bool allowUndo();

private:
    void layoutWidgets();
    void handleToolbar(int action);
    void handleDialogChoice(int button);
    void validateIfWanted();
    string flashMessage;
    float flashTimer;
    sf::Text flashText;

protected:
    void flash(const string& message);     // short feedback line over the canvas
};
