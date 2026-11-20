#include "screens/Level11Traversal.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "game/LevelBuilderB.h"
#include "core/TextUtil.h"

Level11Traversal::Level11Traversal(App& a, const LevelDef& d) : TreeLevelBase(a, d)
{
    expectedCount = 0;
    matched = 0;
    order = ORDER_PRE;
    fontReady = false;
    for (int i = 0; i < Limits::MAX_LEVEL_KEYS; i++)
    {
        expected[i] = 0;
    }
}

string Level11Traversal::orderName()
{
    if (order == ORDER_IN)
    {
        return "inorder";
    }
    if (order == ORDER_POST)
    {
        return "postorder";
    }
    if (order == ORDER_LEVEL)
    {
        return "level order";
    }
    return "preorder";
}

// The expected answer always comes from the model, never from a list typed
// into the level file, so the level cannot disagree with the data structure.
void Level11Traversal::fillExpected()
{
    tree.setEventLog(nullptr);
    if (order == ORDER_IN)
    {
        expectedCount = tree.inorderRec(expected, Limits::MAX_LEVEL_KEYS);
    }
    else if (order == ORDER_POST)
    {
        expectedCount = tree.postorderRec(expected, Limits::MAX_LEVEL_KEYS);
    }
    else if (order == ORDER_LEVEL)
    {
        expectedCount = tree.levelOrder(expected, Limits::MAX_LEVEL_KEYS);
    }
    else
    {
        expectedCount = tree.preorderRec(expected, Limits::MAX_LEVEL_KEYS);
    }
}

void Level11Traversal::onLevelStart()
{
    int pre[Limits::MAX_LEVEL_KEYS];
    int in[Limits::MAX_LEVEL_KEYS];
    LevelBuilderB::buildTree(def, tree, pre, in);

    order = def.k;
    if (order < ORDER_PRE || order > ORDER_LEVEL)
    {
        order = ORDER_PRE;
    }
    fillExpected();
    matched = 0;
    optimalSteps = expectedCount;

    opPanel.clearButtons();
    opPanel.addButton("Show me the patrol");
    opPanel.addButton("Start the route again");
    opPanel.setFieldCount(0);
    opPanel.relayout();

    setObjective("Click the rooms in " + orderName()
                 + " order, the way the patrol will walk them.");
    showTreeStatic(true);
}

void Level11Traversal::onNodeClicked(int nodeId)
{
    if (matched >= expectedCount)
    {
        return;
    }
    if (nodeId != expected[matched])
    {
        // spec penalty table: a wrong traversal click costs 1 alarm
        penalty(1, 0, "Not the next room on the patrol route.");
        return;
    }

    anim.setStateById(nodeId, AnimationController::STATE_VISITED);
    matched++;
    addAlarm(1);
    if (matched >= expectedCount)
    {
        win("You called the patrol route exactly: " + orderName() + " it is.");
    }
}

void Level11Traversal::onOperation(int buttonIndex)
{
    if (buttonIndex == OP_RESTART_ROUTE)
    {
        matched = 0;
        showTreeStatic(false);
        flash("Route cleared. Start again from the first room.");
        return;
    }
    if (buttonIndex != OP_SHOW)
    {
        return;
    }
    // Replaying the real traversal is a hint, so it costs a star the same
    // way the hint button does, and it re-runs the model to get its events.
    hintUsed = true;
    matched = 0;
    int ids[Limits::MAX_LEVEL_KEYS];
    beginOperation();
    if (order == ORDER_IN)
    {
        tree.inorderIter(ids, Limits::MAX_LEVEL_KEYS);
    }
    else if (order == ORDER_POST)
    {
        tree.postorderIter(ids, Limits::MAX_LEVEL_KEYS);
    }
    else if (order == ORDER_LEVEL)
    {
        tree.levelOrder(ids, Limits::MAX_LEVEL_KEYS);
    }
    else
    {
        tree.preorderIter(ids, Limits::MAX_LEVEL_KEYS);
    }
    endOperation(true);
    flash("Watching the patrol costs you a star.");
}

// The tooltip must not hand over the answer, so it shows the room's shape
// rather than its position in the traversal.
string Level11Traversal::tooltipFor(int nodeId)
{
    NodeVisual* v = anim.findById(nodeId);
    if (v == nullptr)
    {
        return "";
    }
    return "Value " + TextUtil::intToText(v->key)
         + "\nDepth " + TextUtil::intToText(v->depth)
         + "\nSubtree rooms " + TextUtil::intToText(v->sizeVal);
}

void Level11Traversal::drawUiOverlay(sf::RenderWindow& window)
{
    if (!fontReady)
    {
        statusText.setFont(app.uiFont);
        statusText.setCharacterSize(Theme::smallSize());
        fontReady = true;
    }
    string line = "Patrol order: " + orderName() + "   |   called "
                + TextUtil::intToText(matched) + " of " + TextUtil::intToText(expectedCount)
                + " rooms";
    statusText.setString(line);
    statusText.setFillColor(Theme::dimText());
    statusText.setPosition(16.f, 16.f);
    window.draw(statusText);
}
