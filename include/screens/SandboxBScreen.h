#pragma once

#include "gui/Screen.h"
#include "gui/Background.h"
#include "gui/Camera.h"
#include "gui/TreeRenderer.h"
#include "gui/AnimationController.h"
#include "gui/HeapAnimator.h"
#include "gui/HeapTreeView.h"
#include "gui/ArrayView.h"
#include "gui/widgets/Button.h"
#include "gui/widgets/EventLogPanel.h"
#include "gui/widgets/OperationPanel.h"
#include "gui/widgets/RoundedRect.h"
#include "gui/widgets/Tooltip.h"
#include "model/BinaryTree.h"
#include "model/MaxHeap.h"

class App;

// The free-form editor for Partner B's two structures, with a tab for each.
// Every operation on the Act 1 binary tree and the Act 4 Max Heap can be run
// here on any data, outside the campaign, which is what the spec asks the
// Sandbox for. The third tab hands over to Partner A's BST / AVL sandbox, so
// all four structures are reachable from one place.
class SandboxBScreen : public Screen
{
public:
    SandboxBScreen(App& app);

    void onEnter();
    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    static const int TAB_TREE = 0, TAB_HEAP = 1, TAB_PARTNER_A = 2;

    // binary tree operations
    static const int T_RANDOM = 0, T_PREORDER_REC = 1, T_PREORDER_ITER = 2,
                     T_INORDER_REC = 3, T_INORDER_ITER = 4, T_POSTORDER_REC = 5,
                     T_POSTORDER_ITER = 6, T_LEVEL_ORDER = 7, T_MIRROR = 8,
                     T_SHAPE_FACTS = 9, T_LCA = 10, T_PATH = 11, T_SYMMETRY = 12;

    // heap operations
    static const int H_RANDOM = 0, H_BUILD = 1, H_INSERT = 2, H_EXTRACT = 3,
                     H_HEAPIFY = 4, H_SIFT_DOWN = 5, H_SIFT_UP = 6, H_HEAP_SORT = 7,
                     H_CHECK = 8, H_RESET = 9;

    App& app;
    Background background;
    Camera camera;

    BinaryTree tree;
    MaxHeap heap;
    EventLog opLog;
    AnimationController treeAnim;
    TreeRenderer treeRenderer;
    HeapAnimator heapAnim;
    HeapTreeView heapView;
    ArrayView arrayView;

    Button tabButtons[3];
    Button backButton;
    OperationPanel opPanel;
    EventLogPanel logPanel;
    RoundedRect factPanel;
    Tooltip tooltip;

    int tab;
    int selectedId;         // tree tab: the selected room
    int selectedIndex;      // heap tab: the selected cell
    int secondId;           // tree tab: the second room, for LCA and paths
    int hoveredId;
    int hoveredIndex;
    int seedCounter;
    string factLines;
    string statusLine;
    sf::Text text;
    float mouseX, mouseY;
    float rightX, rightW, arrayY;
    bool ready;

    void layout();
    void buildOperationPanel();
    void runTreeOperation(int button);
    void runHeapOperation(int button);
    void newRandomTree();
    void newRandomHeap();
    void showTreeStatic();
    void showHeapStatic();
    void beginHeapOperation();
    void endHeapOperation();
    void refreshFacts();
    void setTab(int newTab);
};
