#pragma once

#include "gui/Screen.h"
#include "gui/Background.h"
#include "gui/Camera.h"
#include "gui/HeapAnimator.h"
#include "gui/HeapTreeView.h"
#include "gui/ArrayView.h"
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
#include "model/MaxHeap.h"

class App;

// What the five Act 4 levels share, the same way LevelScreenBase serves
// Acts 2 and 3: briefing dialog, the synchronised heap tree and array views,
// HUD with the alarm meter, operation panel, toolbar, event log, hints,
// undo, replay, the win/lose flow and the call to app.levelFinished.
//
// It is a sibling of LevelScreenBase rather than a subclass of it, because
// that class owns an AVLTree and a heap is an array, not a tree of nodes.
class HeapLevelBase : public Screen
{
public:
    HeapLevelBase(App& app, const LevelDef& def);
    virtual ~HeapLevelBase();

    void onEnter();
    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window);

protected:
    static const int ST_BRIEFING = 0, ST_PLAY = 1, ST_WON = 2, ST_LOST = 3;
    static const int DLG_NONE = 0, DLG_BRIEFING = 1, DLG_WIN = 2, DLG_LOSE = 3, DLG_CUSTOM = 4;

    App& app;
    LevelDef def;
    MaxHeap heap;
    EventLog opLog;
    HeapAnimator anim;
    HeapTreeView heapView;
    ArrayView arrayView;
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
    int selectedIndex, hoveredTreeIndex, hoveredCellIndex;
    float mouseX, mouseY;
    string objective;
    bool animWasFinished;

    // right-hand column, for subclasses that draw their own panel
    float rightX, rightW;

    // ---- services for the subclasses ----
    void setObjective(const string& text);
    void loadHeap(const int values[], int n, int activeHeapSize);
    void showHeapStatic(bool fitCamera);
    void beginOperation();              // snapshot + route events into opLog
    void endOperation(bool autoPlay);   // detach the log and play the animation
    void addAlarm(int steps);
    void penalty(int alarmPlus, int points, const string& message);
    void win(const string& message);
    void lose(const string& message);
    void saveUndo();
    void restoreUndo();
    void fitCameraToHeap();
    void restartLevel();
    void openCustomDialog(const string& title, const string& body,
                          const string labels[], int count);
    LevelOutcome makeOutcome(bool completed);
    void flash(const string& message);
    bool heapIsValid();                 // HeapValidator over the live heap

    // Act 4's player move. The first click selects a cell and the second
    // tries to swap it with the first; an illegal pair costs 1 alarm.
    // Returns true when a swap was actually applied.
    bool selectOrSwap(int index);
    bool performPlayerSwap(int a, int b);
    bool extractOnce();                 // one extract-max, charged 1 alarm

    // ---- hooks each level fills in ----
    virtual void onLevelStart() = 0;    // build the heap, set objective + optimal
    virtual void onCellClicked(int index);
    virtual void onOperation(int buttonIndex);
    virtual void onLevelEvent(const sf::Event& e);
    virtual void onLevelUpdate(float dt);
    virtual void onAnimationDone();
    virtual void onUndoRestored();
    virtual void onDialogChoice(int button);
    virtual void drawUiOverlay(sf::RenderWindow& window);
    virtual bool allowUndo();

private:
    int undoValues[Limits::MAX_NODES_GUI];
    int undoCount;
    int undoHeapSize;
    bool hasUndo;
    int opStartHeapSize;      // active heap size when the operation began

    string flashMessage;
    float flashTimer;
    sf::Text flashText;
    float arrayY, arrayH;

    // a level that asks a question as it starts parks it here until the
    // briefing dialog has been closed, so the briefing cannot swallow it
    bool briefingPending;
    bool pendingDialog;
    string pendingTitle;
    string pendingBody;
    string pendingLabels[Dialog::MAX_DIALOG_BUTTONS];
    int pendingCount;

    void layoutWidgets();
    void handleToolbar(int action);
    void handleDialogChoice(int button);
    void validateIfWanted();
};
