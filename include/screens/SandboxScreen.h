#pragma once

#include "gui/Screen.h"
#include "gui/Background.h"
#include "gui/Camera.h"
#include "gui/TreeRenderer.h"
#include "gui/AnimationController.h"
#include "gui/widgets/Button.h"
#include "gui/widgets/Slider.h"
#include "gui/widgets/Toolbar.h"
#include "gui/widgets/EventLogPanel.h"
#include "gui/widgets/OperationPanel.h"
#include "gui/widgets/Tooltip.h"
#include "model/AVLTree.h"

class App;

// The free-form editor (spec 2.3): tabs Binary | BST | AVL | MaxHeap.
// Partner A owns the BST and AVL tabs with every operation; the Binary
// and MaxHeap tabs show "coming soon" until Partner B fills their classes.
class SandboxScreen : public Screen
{
public:
    SandboxScreen(App& app);
    ~SandboxScreen();

    void onEnter();
    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    static const int TAB_BINARY = 0, TAB_BST = 1, TAB_AVL = 2, TAB_HEAP = 3;
    static const int UNDO_DEPTH = 16;

    // operation button ids (indices into the OperationPanel)
    static const int OP_INSERT = 0, OP_DELETE = 1, OP_SEARCH = 2, OP_RANGE = 3,
                     OP_MIN = 4, OP_MAX = 5, OP_SUCC = 6, OP_PRED = 7,
                     OP_KTH = 8, OP_RANK = 9, OP_ROT_LEFT = 10, OP_ROT_RIGHT = 11,
                     OP_AUTOREBALANCE = 12, OP_RANDOM = 13, OP_CLEAR = 14, OP_UNDO = 15;

    App& app;
    Background background;
    Camera camera;
    TreeRenderer renderer;
    AnimationController anim;
    Toolbar toolbar;
    EventLogPanel logPanel;
    OperationPanel opPanel;
    Tooltip tooltip;
    Button tabButtons[4];
    Slider sizeSlider;
    sf::Text titleText;
    sf::Text statusText;
    sf::Text comingSoonText;
    sf::Text sizeLabel;

    int tab;
    AVLTree bstTree;       // auto-rebalance permanently off
    AVLTree avlTree;
    EventLog opLog;
    AVLTree* undoStack[UNDO_DEPTH];
    int undoCount;
    int selectedId;
    int hoveredId;
    int badgeMode;
    float mouseX, mouseY;
    int randomSeed;
    string status;
    bool widgetsReady;

    AVLTree& activeTree();
    bool treeTabActive();
    void layoutWidgets();
    void setTab(int newTab);
    void rebuildOpButtons();
    void showStatic(bool fit);
    void beginOp();
    void endOp();
    void saveUndoCopy();
    void doUndo();
    void clearUndoStack();
    void runOperation(int op);
    void randomTree();
    void setStatus(const string& text);
};
