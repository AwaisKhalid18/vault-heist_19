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
#include "gui/widgets/OperationPanel.h"
#include "gui/widgets/HintPanel.h"
#include "gui/widgets/Dialog.h"
#include "gui/widgets/Tooltip.h"
#include "game/LevelDef.h"
#include "game/ScoreSystem.h"
#include "model/BinaryTree.h"

class App;

// What the five Act 1 levels share. It is the same shape as Partner A's
// LevelScreenBase, but it owns a BinaryTree instead of an AVLTree, because
// Act 1's vaults carry arbitrary values in arbitrary shapes with no ordering
// rule at all.
class TreeLevelBase : public Screen
{
public:
    TreeLevelBase(App& app, const LevelDef& def);
    virtual ~TreeLevelBase();

    void onEnter();
    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window);

protected:
    static const int ST_BRIEFING = 0, ST_PLAY = 1, ST_WON = 2, ST_LOST = 3;
    static const int DLG_NONE = 0, DLG_BRIEFING = 1, DLG_WIN = 2, DLG_LOSE = 3, DLG_CUSTOM = 4;

    App& app;
    LevelDef def;
    BinaryTree tree;
    EventLog opLog;
    AnimationController anim;
    TreeRenderer renderer;
    Camera camera;
    Background background;
    ParticleBurst particles;
    Hud hud;
    Toolbar toolbar;
    EventLogPanel logPanel;
    OperationPanel opPanel;
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
    bool animWasFinished;
    bool showTooltips;

    float rightX, rightW;

    // ---- services for the subclasses ----
    void setObjective(const string& text);
    void showTreeStatic(bool fitCamera);
    void beginOperation();
    void endOperation(bool autoPlay);
    void addAlarm(int steps);
    void penalty(int alarmPlus, int points, const string& message);
    void win(const string& message);
    void lose(const string& message);
    void fitCameraToTree();
    void restartLevel();
    void openCustomDialog(const string& title, const string& body,
                          const string labels[], int count);
    LevelOutcome makeOutcome(bool completed);
    void flash(const string& message);
    TreeNode* nodeById(int id);

    // ---- hooks each level fills in ----
    virtual void onLevelStart() = 0;
    virtual void onNodeClicked(int nodeId);
    virtual void onOperation(int buttonIndex);
    virtual void onLevelEvent(const sf::Event& e);
    virtual void onLevelUpdate(float dt);
    virtual void onAnimationDone();
    virtual void onDialogChoice(int button);
    virtual void drawCanvasOverlay(sf::RenderWindow& window);
    virtual void drawUiOverlay(sf::RenderWindow& window);
    virtual string tooltipFor(int nodeId);

private:
    string flashMessage;
    float flashTimer;
    sf::Text flashText;

    bool briefingPending;
    bool pendingDialog;
    string pendingTitle;
    string pendingBody;
    string pendingLabels[Dialog::MAX_DIALOG_BUTTONS];
    int pendingCount;

    void layoutWidgets();
    void handleToolbar(int action);
    void handleDialogChoice(int button);
};
