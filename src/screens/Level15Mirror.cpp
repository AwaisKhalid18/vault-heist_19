#include "screens/Level15Mirror.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "game/LevelBuilderB.h"
#include "core/TextUtil.h"
#include "core/NodeStack.h"

// How many rooms actually have two different children to swap. A room with no
// children, or with only one, still has to be "turned", but a vault where
// every room is a leaf needs no work at all, so this is the honest optimum.
static int countSwappableRooms(TreeNode* root)
{
    int count = 0;
    NodeStack stack;
    if (root != nullptr)
    {
        stack.push(root);
    }
    while (!stack.isEmpty())
    {
        TreeNode* node = stack.pop();
        if (node->left != nullptr || node->right != nullptr)
        {
            count++;
        }
        if (node->right != nullptr)
        {
            stack.push(node->right);
        }
        if (node->left != nullptr)
        {
            stack.push(node->left);
        }
    }
    return count;
}

Level15Mirror::Level15Mirror(App& a, const LevelDef& d) : TreeLevelBase(a, d)
{
    roomCount = 0;
    swapsMade = 0;
    fontReady = false;
    for (int i = 0; i < Limits::MAX_LEVEL_KEYS; i++)
    {
        startPre[i] = 0;
        startIn[i] = 0;
    }
}

void Level15Mirror::rebuildStart()
{
    tree.buildPreIn(startPre, startIn, roomCount);
}

void Level15Mirror::onLevelStart()
{
    roomCount = LevelBuilderB::buildTree(def, tree, startPre, startIn);

    // the goal is the same vault, mirrored - built by the model, so the win
    // check is a structural comparison and not a rule typed into the screen
    goal.buildPreIn(startPre, startIn, roomCount);
    goal.setEventLog(nullptr);
    goal.mirror();

    swapsMade = 0;
    selectedId = -1;
    optimalSteps = countSwappableRooms(tree.getRoot());
    if (optimalSteps < 1)
    {
        optimalSteps = 1;
    }

    opPanel.clearButtons();
    opPanel.addButton("Auto: mirror()");
    opPanel.addButton("Put the vault back");
    opPanel.setFieldCount(0);
    opPanel.relayout();

    setObjective("Click a room to swap its two children. Mirror the whole "
                 "vault, working from the bottom up.");
    showTreeStatic(true);
}

void Level15Mirror::onNodeClicked(int nodeId)
{
    TreeNode* node = nodeById(nodeId);
    if (node == nullptr)
    {
        return;
    }
    if (node->left == nullptr && node->right == nullptr)
    {
        flash("A room with no corridors below it has nothing to swap.");
        return;
    }

    // The swap is a shape change, so the animation needs a snapshot before
    // and after; the animator then slides every room by id to its new place.
    opLog.clear();
    opLog.saveTreeShape(tree.getRoot());

    TreeNode* hold = node->left;
    node->left = node->right;
    node->right = hold;

    Event e;
    e.type = Event::ROTATE;
    e.nodeId = node->id;
    e.otherId = node->id;
    e.text = "Swap the children of room " + TextUtil::intToText(node->key);
    e.snapshotIndex = opLog.saveTreeShape(tree.getRoot());
    opLog.add(e);
    Event done;
    done.type = Event::DONE;
    done.text = "Room turned";
    opLog.add(done);

    anim.setSpeed(toolbar.getSpeed());
    anim.startLog(&opLog, true);
    animWasFinished = false;

    swapsMade++;
    addAlarm(1);
}

void Level15Mirror::onOperation(int buttonIndex)
{
    if (buttonIndex == OP_RESET_SHAPE)
    {
        rebuildStart();
        swapsMade = 0;
        showTreeStatic(false);
        flash("The vault is back the way you found it.");
        return;
    }
    if (buttonIndex != OP_AUTO)
    {
        return;
    }
    // one call to the model's recursive mirror(), charged for the rooms it
    // turns so it is never cheaper than doing the work by hand
    int cost = countSwappableRooms(tree.getRoot());
    beginOperation();
    tree.mirror();
    endOperation(true);
    swapsMade += cost;
    addAlarm(cost);
}

void Level15Mirror::onAnimationDone()
{
    checkForWin();
}

void Level15Mirror::checkForWin()
{
    if (tree.isSameTree(goal))
    {
        win("The vault is its own reflection now - every left is a right.");
    }
}

void Level15Mirror::drawUiOverlay(sf::RenderWindow& window)
{
    if (!fontReady)
    {
        lineText.setFont(app.uiFont);
        lineText.setCharacterSize(Theme::smallSize());
        fontReady = true;
    }
    lineText.setString("Rooms turned " + TextUtil::intToText(swapsMade)
                       + "   |   rooms worth turning " + TextUtil::intToText(optimalSteps)
                       + "   |   click a room to swap its two corridors");
    lineText.setFillColor(Theme::dimText());
    lineText.setPosition(16.f, 16.f);
    window.draw(lineText);
}
